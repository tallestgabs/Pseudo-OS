#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>    // Necessário para vector
#include <sstream>   // Importante para ler os dados da linha
#include <tuple>     // Necessário para usar o std::tie
#include <algorithm> // Necessário para o sort

#include "../include/Process.hpp"
#include "../include/QueueManager.hpp"
#include "../include/ResourceManager.hpp"

using namespace std;


vector<Process> create_process(string process)
{
    ifstream myFile(process);
    if(!myFile.is_open())
    {
        cerr << "Error ! Não foi possível abrir o arquivo " << process << "\n";
        exit(EXIT_FAILURE);
    }

    // vetor local
    vector<Process> local_queue;

    string line;
    int current_id = 0; 

    

    while(getline(myFile, line))
    {
        // Ignora linhas vazias
        if(line.empty()) continue;

        stringstream ss(line);
        char virgula;
        
        Process new_process;
        new_process.pid = current_id++;

        if (ss >> new_process.init_time >> virgula
               >> new_process.priority >> virgula
               >> new_process.cpu_time >> virgula
               >> new_process.frames >> virgula
               >> new_process.printer_req >> virgula
               >> new_process.scanner_req >> virgula
               >> new_process.modem_req >> virgula
               >> new_process.sata_req) 
        {
            
            local_queue.push_back(new_process);
        }
    }
    // Ordenando pela ordem de chegada em caso de empate pela prioridade
    std::sort(local_queue.begin(), local_queue.end(), [](const Process& a, const Process& b) {  // sem std tava dando sort is ambiguous
    return tie(a.init_time, a.priority) < 
           tie(b.init_time, b.priority);
	});

    myFile.close();

    // retorna a lista para a main
    return local_queue;
}

int main(int argc, char* argv[])
{
    if(argc < 4)
    {
        cout << "Uso: ./main processes.txt files.txt memory.txt\n";
        return 1;
    }

    // coloca o resultado de create_process em read_processes (processos lidos)
    vector<Process> read_processes = create_process(argv[1]);

    // cria o gerenciador de filas passando processos lidos
    QueueManager queue_manager(read_processes);

	// cria o gerenciador de arquivos e dispositivos de I/O
	ResourceManager resource_manager(argv[2]);
	/*========================= TESTE ================================*/
    for(std::tuple t: resource_manager.getOccupiedSegmentsInstructions()) {
    	cout << std::get<0>(t) << " " << std::get<1>(t) << " " << std::get<2>(t) << "\n";
    }

    // clock da CPU
    int system_clock = 0;

    // LOOP PRINCIPAL DO SO
    while(queue_manager.has_readyProcesses()){
        // verifica se no clock atual chegou um processo novo
        queue_manager.verify_arrivals(system_clock);
        
        // pede o processo de maior prioridade
        Process* current_process = queue_manager.get_next_process();

        if(current_process != nullptr){
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

            // Separa execucao
            if(current_process->priority == 0){
                // Tempo Real -> FIFO, Sem Preempcao 
                while(current_process->cpu_time > 0){
                    current_process->pc++;  
                    cout << "P" << current_process->pid << " instruction " << current_process->pc << "\n";

                    current_process->cpu_time--;
                    system_clock++;
                }
                cout << "P" << current_process->pid << " return SIGINT\n\n";
                delete current_process;
            }
            else{
                // Usuario -> Preempcao, Quantum de 1ms
                current_process->pc++;  
                cout << "P" << current_process->pid << " instruction " << current_process->pc << "\n";

                current_process->cpu_time--;
                system_clock++;

                // verifica se terminou antes do 1ms
                if(current_process->cpu_time <= 0){
                    cout << "P" << current_process->pid << " return SIGINT\n\n";
                    delete current_process;
                } else{
                    // se nao terminou vai sofrer preempcao e aging (prioridade abaixa)
                    queue_manager.reallocate_process(current_process);
                }
            }
            
        }
        else{
            // se nao tiver ninguem a cpu fica ociosa e o clock avança
            system_clock++;
        }
    }
    return 0;
}
