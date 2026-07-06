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
#include "../include/FileSystem.hpp"
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

        if (ss >> new_process->init_time >> virgula
               >> new_process->priority >> virgula
               >> new_process->cpu_time >> virgula
               >> new_process->frames >> virgula
               >> new_process->printer_req >> virgula
               >> new_process->scanner_req >> virgula
               >> new_process->modem_req >> virgula
               >> new_process->sata_req)
        {

            // elimina processos com dados incorretos (i.e: prioridade > 3, printer, scanner, modem, sata maiores que o definido)
            if(new_process->priority > 3 ||
               new_process->priority < 0 ||
               new_process->printer_req > ResourceManager::max_printers||
               new_process->scanner_req > ResourceManager::max_scanners||
               new_process->modem_req > ResourceManager::max_modems||
               new_process->sata_req > ResourceManager::max_satas){

                // deleta processo
                delete new_process;

                // vai para o proximo processo
                continue;

            } 
            
            // se os valores são válidos então adiciona no vetor de processos

            new_process->pid = current_id++;       // -> em vez de .
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
    FileSystem resource_manager(argv[2]);
    resource_manager.execSegmentsInstruction();

    // Liga cada processo às suas instruções ANTES de ordenar,
    // usando o pid como índice em unsorted_processes (que preserva a ordem original)
    for (tuple t : resource_manager.getProcessInstructions())
    {
        long pid = std::get<0>(t);
        if (pid < 0 || pid >= (long)unsorted_processes.size())
            continue;
        else
        	unsorted_processes[pid]->process_instructions.push_back(t); // -> em vez de .
    }

    // Executa TODAS as operações de arquivo em bloco (na ordem original do
    // arquivo de recursos) e guarda o log para impressão depois do loop da CPU
    resource_manager.execAllProcessInstructions(unsorted_processes);

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

    // Resource Manager (Gerenciado de Dispositivos E/S)
    ResourceManager io_manager;

    // clock da CPU
    int system_clock = 0;

    // LOOP PRINCIPAL DO SO
    while (queue_manager.has_readyProcesses())
    {
        queue_manager.verify_arrivals(system_clock);

        // aplica o Aging para evitar starvation
        queue_manager.apply_aging();

        Process* current_process = queue_manager.get_next_process();

        if (current_process != nullptr)
        {
            // Processos de usuario que ainda nao pegaram hardware 
            if(current_process->priority > 0 && !current_process->has_resources){
                if(io_manager.verify_andAllocate(current_process)){
                    // conseguiu pegar o hardware
                    current_process->has_resources = true;
                }
                else{
                    // falha! nao ha quantidade de recursos solicitados
                    // devolve para a mesma fila sem penalidade e tenta novamente no proximo quantum
                    queue_manager.reallocate_process_no_penalty(current_process);
                    system_clock++;
                    continue;           // pula o resto e vai para a proxima iteracao
                }
            }

    //--------------------------------------------------------------------------
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

                    // NOVO: Execução da memória c/ verificação de eliminação (Tempo Real)
                    if (!memory_manager.execute_instruction(current_process)) {
                        cout << "P" << current_process->pid << " ELIMINADO: Pediu " << current_process->frames 
                             << " frames, superando a memoria livre.\n";
                        current_process->cpu_time = 0; // Aborta e força o encerramento do while
                        break;
                    }

                    current_process->cpu_time--;
                    system_clock++;
                }
                cout << "P" << current_process->pid << " return SIGINT\n\n";

                page_faults_summary[current_process->pid] = current_process->page_faults;
                memory_manager.terminate_process(current_process);
            }
            else
            {
                current_process->pc++;
                cout << "P" << current_process->pid << " instruction " << current_process->pc << "\n";
                
                // NOVO: Execução da memória c/ verificação de eliminação (Usuário)
                if (!memory_manager.execute_instruction(current_process)) {
                    cout << "P" << current_process->pid << " ELIMINADO: Pediu " << current_process->frames 
                         << " frames, superando a memoria livre.\n";
                    current_process->cpu_time = 0; // Zera para encerrar imediatamente abaixo
                } else {
                    current_process->cpu_time--;
                    system_clock++;
                }

                if (current_process->cpu_time <= 0)
                {
                    cout << "P" << current_process->pid << " return SIGINT\n\n";

                    io_manager.free_resources(current_process);

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

    // imprime o log das operações do sistema de arquivos, montado em bloco
    // por resource_manager.execAllProcessInstructions()
    cout << "Sistema de arquivos =>\n";
    for (const std::string& line : resource_manager.file_system_log)
        cout << line << "\n";
    cout << "\n";

    cout << "Mapa de ocupação do disco:\n";
	for (pair p : resource_manager.getDisk())
        cout << p.first << " ";
    cout << "\n\n";

    // printa o resumo das faltas de página por processo
    cout << "Número de Faltas de Páginas por processo:\n";
    for (auto const& [pid, faults] : page_faults_summary) {
        cout << "P" << pid << " = " << faults << " faltas de páginas\n";
    }

    return 0;
}
