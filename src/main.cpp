#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <tuple>
#include <algorithm>
#include <map>

#include "../include/MemoryManager.hpp"
#include "../include/Process.hpp"
#include "../include/QueueManager.hpp"
#include "../include/ResourceManager.hpp"

using namespace std;

// Agora só cria a lista de processos na ORDEM ORIGINAL do arquivo (pid = índice)
vector<Process*> create_process(string process)
{
    ifstream myFile(process);
    if (!myFile.is_open())
    {
        cerr << "Error ! Não foi possível abrir o arquivo " << process << "\n";
        exit(EXIT_FAILURE);
    }

    vector<Process*> processes;

    string line;
    int current_id = 0;

    while (getline(myFile, line))
    {
        if (line.empty()) continue;

        stringstream ss(line);
        char virgula;

        Process* new_process = new Process(); // heap, sobrevive além do loop
        new_process->pid = current_id++;       // -> em vez de .

        if (ss >> new_process->init_time >> virgula
               >> new_process->priority >> virgula
               >> new_process->cpu_time >> virgula
               >> new_process->frames >> virgula
               >> new_process->printer_req >> virgula
               >> new_process->scanner_req >> virgula
               >> new_process->modem_req >> virgula
               >> new_process->sata_req)
        {
            processes.push_back(new_process);
        }
        else
        {
            // linha inválida/malformada: descarta pra não deixar lixo/pid furado
            delete new_process;
        }
    }

    myFile.close();
    return processes;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        cout << "Uso: ./main processes.txt files.txt memory.txt\n";
        return 1;
    }

    // Lê os processos NA ORDEM ORIGINAL do arquivo (pid == índice no vetor)
    vector<Process*> unsorted_processes = create_process(argv[1]);

    // cria o gerenciador de arquivos e dispositivos de I/O
    ResourceManager resource_manager(argv[2]);

    // Liga cada processo às suas instruções ANTES de ordenar,
    // usando o pid como índice em unsorted_processes (que preserva a ordem original)
    for (auto t : resource_manager.getProcessInstructions())
    {
        long pid = std::get<0>(t);
        if (pid < 0 || pid >= (long)unsorted_processes.size())
            continue;
        unsorted_processes[pid]->process_instructions.push_back(t); // -> em vez de .
    }

    for (int i : resource_manager.getDisk())
        cout << i << " ";
    cout << "\n";

    for (Process* p : unsorted_processes)     // Process* em vez de Process
    {
        if (!p->process_instructions.empty())
            cout << get<0>(p->process_instructions[0]) << " ";
    }
    cout << "\n";

    // Só agora monta a fila ORDENADA (por init_time, priority),
    // já copiando processos que já têm as instruções ligadas
    vector<Process> sorted_queue;
    sorted_queue.reserve(unsorted_processes.size());
    for (Process* p : unsorted_processes)
        sorted_queue.push_back(*p); // copia já inclui process_instructions

    std::sort(sorted_queue.begin(), sorted_queue.end(),
        [](const Process& a, const Process& b) {
            return tie(a.init_time, a.priority) < tie(b.init_time, b.priority);
        });

    // inicializacao do gerenciador de memoria
    MemoryManager memory_manager;
    memory_manager.load_references(argv[3], sorted_queue);
    // mapa para o backup das faltas de página
    std::map<int, int> page_faults_summary;

    // cria o gerenciador de filas já com os processos ordenados e ligados às instruções
    QueueManager queue_manager(sorted_queue);

    // clock da CPU
    int system_clock = 0;

    // LOOP PRINCIPAL DO SO
    while (queue_manager.has_readyProcesses())
    {
        queue_manager.verify_arrivals(system_clock);

        Process* current_process = queue_manager.get_next_process();

        if (current_process != nullptr)
        {
            cout << "dispatcher =>\n";
            cout << "	PID: " << current_process->pid << "\n";
            cout << "	frames: " << current_process->frames << "\n";
            cout << "	priority: " << current_process->priority << "\n";
            cout << "	time: " << current_process->cpu_time << "\n";
            cout << "	printers: " << current_process->printer_req << "\n";
            cout << "	scanners: " << current_process->scanner_req << "\n";
            cout << "	modems: " << current_process->modem_req << "\n";
            cout << "	drives: " << current_process->sata_req << "\n";

            cout << "process " << current_process->pid << " =>\n";
            cout << "P" << current_process->pid << " STARTED\n";

            if (current_process->priority == 0)
            {
                while (current_process->cpu_time > 0)
                {
                    current_process->pc++;
                    cout << "P" << current_process->pid << " instruction " << current_process->pc << "\n";
                    resource_manager.execInstruction(current_process);

                    // execucao da memoria em tempo real (prioridade 0)
                    memory_manager.execute_instruction(current_process);
                    // ===================================================

                    current_process->cpu_time--;
                    system_clock++;
                }
                cout << "P" << current_process->pid << " return SIGINT\n\n";

                // encerramento do processo de tempo real (prioridade 0) e liberacao da memoria
                page_faults_summary[current_process->pid] = current_process->page_faults;
                memory_manager.terminate_process(current_process);
            }
            else
            {
                current_process->pc++;
                cout << "P" << current_process->pid << " instruction " << current_process->pc << "\n";
                
                // execucao da memoria em tempo de usuario (prioridade 1, 2 ou 3)
                memory_manager.execute_instruction(current_process);

                current_process->cpu_time--;
                system_clock++;

                if (current_process->cpu_time <= 0)
                {
                    cout << "P" << current_process->pid << " return SIGINT\n\n";

                    // encerramento do processo de usuario (prioridade 1, 2 ou 3) e liberacao da memoria
                    page_faults_summary[current_process->pid] = current_process->page_faults;
                    memory_manager.terminate_process(current_process);
                }
                else
                {
                    queue_manager.reallocate_process(current_process);
                }
            }
        }
        else
        {
            system_clock++;
        }
    }

    // libera a memória alocada em create_process
    for (Process* p : unsorted_processes)
        delete p;

    // printa o resumo das faltas de página por processo
    cout << "Número de Faltas de Páginas por processo:\n";
    for (auto const& [pid, faults] : page_faults_summary) {
        cout << "P" << pid << " = " << faults << " faltas de páginas\n";
    }

    return 0;
}
