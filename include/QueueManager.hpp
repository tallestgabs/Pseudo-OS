#ifndef QUEUEMANAGER_HPP
#define QUEUEMANAGER_HPP

#include <queue>
#include <vector>
#include "Process.hpp"


class QueueManager{
    private:
        // lista global ordenada por tempo de chegada
        std::vector<Process> global_processes_queue;

        // filas de execução
        std::queue<Process*> realTime_queue;  // prioridade 0
        std::queue<Process*> user1_queue;     // prioridade 1
        std::queue<Process*> user2_queue;     // prioridade 2
        std::queue<Process*> user3_queue;     // prioridade 3

    public:
        // recebe o vetor do parser
        QueueManager(const std::vector<Process>& read_processes); // processos LIDOS

        // verifica cada tique do clock
        void verify_arrivals(int current_clock);

        // escolhe o novo processo que vai assumir a cpu
        Process* get_next_process();

        // Verifica que há processo em alguma fila
        bool has_readyProcesses() const;


        void reallocate_process(Process* p);

        // getter (DEBUG se precisar)
        const std::vector<Process>& get_global_processes_queue() const;

};


#endif