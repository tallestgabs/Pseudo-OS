#ifndef MEMORYMANAGER_HPP
#define MEMORYMANAGER_HPP

#include "Process.hpp"
#include <list>
#include <unordered_map>
#include <string>
#include <vector>

class MemoryManager {
private:
    int free_rt_frames; // 8 frames para processos de tempo real
    int free_user_frames; // 12 frames para processos de usuário

    // Estrutura para manter o estado da memória de cada processo localmente
    struct ProcessMemory {
        std::list<int> pages; // aqui é o LRU
        int allocated_frames = 0; // frames atuais
        size_t current_ref_index = 0; // ponteiro pro proximo frame
    };

    std::unordered_map<int, ProcessMemory> memory_table; // mapa para o PID do processo e a memória

public:
    MemoryManager();

    // le o arquivo de referencias e associa ao processo certo
    void load_references(const std::string& filename, std::vector<Process>& processes);

    // processa algumas instrucoes durante o ciclo de CPU e atualiza a tabela de memoria do processo
    void execute_instruction(Process* p);

    // libera os frames já usados e remove da tabela da memoria do processo
    void terminate_process(Process* p);
};

#endif