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

    p->waiting_time = 0;  // acabou de sair da CPU

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


void QueueManager::apply_aging(){
    // promove processos com muito tempo na fila 2
    int queue2_size = user2_queue.size();
    for(int i = 0; i < queue2_size; i++){
        Process* p = user2_queue.front();
        user2_queue.pop();

        p->waiting_time++;   // envelheceu

        if(p->waiting_time >= 2){   // momento de promoção
            p->priority = 1;        // sobe para a fila 1
            p->waiting_time = 0;    // zera a espera
            user1_queue.push(p);    // vai para a fila 1
        }
        else{
            user2_queue.push(p);    // volta para a fila 2
        }
    }
    // faz a mesma coisa para a fila 3     
    int queue3_size = user3_queue.size();
    for(int i = 0; i < queue3_size; i++){
        Process* p = user3_queue.front();
        user3_queue.pop();

        p->waiting_time++;   

        if(p->waiting_time >= 2){   
            p->priority = 2;        
            p->waiting_time = 0;    
            user2_queue.push(p);    
        }
        else{
            user3_queue.push(p);    
        }
    }
}    



