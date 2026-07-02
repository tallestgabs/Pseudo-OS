#include "../include/QueueManager.hpp"

// inicia construtor
QueueManager::QueueManager(const std::vector<Process>& read_processes){
    this->global_processes_queue = read_processes;
}

// simula hardware e olha quem chegou no tempo atual
void QueueManager::verify_arrivals(int current_clock){
    // vetor já foi ordenado então olhamos o primeiro elemento
    while (!global_processes_queue.empty() && global_processes_queue.front().init_time <= current_clock){
        // pega o processo que chegou no tempo atual
        Process* new_process = new Process(global_processes_queue.front());
        // remove da fila global
        global_processes_queue.erase(global_processes_queue.begin());

        // coloca nas filas corretas
        switch (new_process->priority){
            case 0:
                realTime_queue.push(new_process);
                break;
            case 1:
                user1_queue.push(new_process);
                break;

            case 2:
                user2_queue.push(new_process);
                break;

            default:
                user3_queue.push(new_process);
                break;
        }

    }
}

Process* QueueManager::get_next_process(){
    // Está em ordem hierarquica

    if(!realTime_queue.empty()){
        Process* p = realTime_queue.front();
        realTime_queue.pop();
        return p;
    }

    if(!user1_queue.empty()){
        Process* p = user1_queue.front();
        user1_queue.pop();
        return p;
    }

    if(!user2_queue.empty()){
        Process* p = user2_queue.front();
        user2_queue.pop();
        return p;
    }

    if(!user3_queue.empty()){
        Process* p = user3_queue.front();
        user3_queue.pop();
        return p;
    }

    return nullptr;  // se nao tiver processo para rodar a CPU não faz nada
}



bool QueueManager::has_readyProcesses() const{
    return  !realTime_queue.empty() ||
            !user1_queue.empty() ||
            !user2_queue.empty() ||
            !user3_queue.empty() ||
            !global_processes_queue.empty();
}


void QueueManager::reallocate_process(Process* p){
    // quem acabou de usar a cpu eh penalizado abaixando sua prioridade
    // quando menor o numero, maior prioridade no escalonador
    if(p->priority < 3){
        p->priority++;  // prioridade diminui
    }

    // coloca o processo na fila correspondente a sua prioridade
    if(p->priority == 1){
        user1_queue.push(p);
    }
    else if(p->priority == 2){
        user2_queue.push(p);
    }
    else{
        user3_queue.push(p);
    }
    }


// getter
const std::vector<Process>& QueueManager::get_global_processes_queue() const{
    return global_processes_queue;
}