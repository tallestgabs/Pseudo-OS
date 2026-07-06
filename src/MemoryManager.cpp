#include "../include/MemoryManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

MemoryManager::MemoryManager() {
    this->free_rt_frames = 8; // 8 frames exclusivos para Tempo Real
    this->free_user_frames = 12; // 12 frames exclusivos para Usuários
}

void MemoryManager::load_references(const std::string& filename, std::vector<Process>& processes) {
    std::ifstream file(filename); 
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir " << filename << "\n"; 
        exit(EXIT_FAILURE);
    }

    std::string line;
    int pid_counter = 0; 
    while (std::getline(file, line)) { 
        if (line.empty()) continue;

        std::stringstream ss(line); 
        std::string token;
        std::vector<int> refs;
        while (std::getline(ss, token, ',')) {
            refs.push_back(std::stoi(token));
        }

        for (auto& p : processes) {
            if (p.pid == pid_counter) {
                p.references = refs;
                break;
            }
        }
        pid_counter++; 
    }
    file.close();
}

bool MemoryManager::execute_instruction(Process* p) {
    auto& p_mem = memory_table[p->pid];
    bool is_rt = (p->priority == 0);
    int& global_free = is_rt ? free_rt_frames : free_user_frames;
    
    // Capacidade absoluta máxima de cada partição
    int max_allowed_partition = is_rt ? 8 : 12;

    // INICIALIZAÇÃO E PRÉ-CARGA
    if (!p_mem.preloaded) {
        
        // CORREÇÃO: Elimina APENAS se o processo pedir mais frames do que 
        // a capacidade máxima absoluta da partição dele. (ex: P4 pede 16, limite é 12 -> Morre)
        if (p->frames > max_allowed_partition) {
            return false; // Retorna falso avisando a main() para matar o processo
        }

        // PRÉ-CARGA DE 1 PÁGINA: Isso corrige a "1 falta de página a mais".
        // Garante que a primeira instrução encontre a página na memória gerando um Hit.
        if (!p->references.empty() && p->frames > 0 && global_free > 0) {
            int first_page = p->references[0];
            p_mem.pages.push_back(first_page);
            p_mem.allocated_frames++;
            global_free--;
        }
        p_mem.preloaded = true; // Marca que já foi feito para não repetir
    }
    
    // Se o processo não tem referências, ou já leu todas, apenas retorna sucesso
    if (p->references.empty() || p_mem.current_ref_index >= p->references.size()) {
        return true;
    }

    // Calcula quantas instruções de memória rodar neste ciclo da CPU
    int remaining_refs = p->references.size() - p_mem.current_ref_index;
    int refs_to_process = 1; 

    if (p->cpu_time == 1) { 
        refs_to_process = remaining_refs; 
    } else if (p->cpu_time > 1) {
        refs_to_process = remaining_refs / p->cpu_time; 
    }

    // Loop executando as referências
    for (int i = 0; i < refs_to_process && p_mem.current_ref_index < p->references.size(); i++) {
        int page = p->references[p_mem.current_ref_index++];
        
        // Procura a página atual na memória local do processo
        auto it = std::find(p_mem.pages.begin(), p_mem.pages.end(), page);
        
        if (it != p_mem.pages.end()) {
            // HIT (Acerto): A pré-carga garante que a primeira página caia aqui!
            p_mem.pages.erase(it);
            p_mem.pages.push_back(page); // Move pro final (MRU - mais recentemente usada)
        } else {
            // PAGE FAULT (Falta de página)
            p->page_faults++;
            
            // Se o processo ainda não atingiu o limite dele e tem memória livre no sistema
            if (p_mem.allocated_frames < p->frames && global_free > 0) {
                p_mem.allocated_frames++;
                global_free--;
                p_mem.pages.push_back(page);
            } 
            // Se já atingiu o limite, aplica o algoritmo LRU Local
            else if (p->frames > 0) {
                if (!p_mem.pages.empty()) {
                    p_mem.pages.pop_front(); // Remove a página na frente (LRU - menos recentemente usada)
                }
                p_mem.pages.push_back(page); // Insere a nova
            }
        }
    }
    return true; // Executou com sucesso
}

void MemoryManager::terminate_process(Process* p) {
    auto it = memory_table.find(p->pid);
    if (it != memory_table.end()) {
        // Devolve os frames que o processo usou para o sistema
        bool is_rt = (p->priority == 0);
        if (is_rt) {
            free_rt_frames += it->second.allocated_frames;
        } else {
            free_user_frames += it->second.allocated_frames;
        }
        memory_table.erase(it); // Apaga a memória do processo
    }
}