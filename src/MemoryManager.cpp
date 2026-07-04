#include "../include/MemoryManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

MemoryManager::MemoryManager() {
    this->free_rt_frames = 8; // 8 frames para processos de tempo real
    this->free_user_frames = 12; // 12 frames para processos de usuário
}

void MemoryManager::load_references(const std::string& filename, std::vector<Process>& processes) {
    std::ifstream file(filename); // tenta abrir o arquivo de referencias
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir " << filename << "\n"; // retorna erro se nao conseguir abrir o arquivo
        exit(EXIT_FAILURE);
    }

    std::string line;
    int pid_counter = 0; // contador de PID
    while (std::getline(file, line)) { 
        if (line.empty()) continue;

        std::stringstream ss(line); // transforma a linha em stringstream
        std::string token;
        std::vector<int> refs;
        while (std::getline(ss, token, ',')) {
            refs.push_back(std::stoi(token));
        }

        // linka a linha do arquivo ao processo usando o PID
        for (auto& p : processes) {
            if (p.pid == pid_counter) {
                p.references = refs;
                break;
            }
        }
        pid_counter++; //incrementa o PID para o proximo processo
    }
    file.close();
}

void MemoryManager::execute_instruction(Process* p) {
    auto& p_mem = memory_table[p->pid];
    
    // se consumiu todas as paginas, encerra
    if (p->references.empty() || p_mem.current_ref_index >= p->references.size()) {
        return;
    }

    // processa as referencias de acordo com o tCPU restante do processo
    int remaining_refs = p->references.size() - p_mem.current_ref_index;
    int refs_to_process = 1; 

    // Se so 1 ciclo de cpu, processa todo o resto das referencias
    if (p->cpu_time == 1) { 
        refs_to_process = remaining_refs; // ultimo ciclo da cpu, consome tudo
    } else if (p->cpu_time > 1) {
        refs_to_process = remaining_refs / p->cpu_time; 
    }

    for (int i = 0; i < refs_to_process && p_mem.current_ref_index < p->references.size(); i++) {
        int page = p->references[p_mem.current_ref_index++];
        
        // procura a pagina na lista de páginas do processo (LRU)
        auto it = std::find(p_mem.pages.begin(), p_mem.pages.end(), page);
        
        if (it != p_mem.pages.end()) {
            // Hit (Página encontrada). Move para o final (Most Recently Used)
            p_mem.pages.erase(it);
            p_mem.pages.push_back(page);
        } else {
            // Page Fault (Falta de página)
            p->page_faults++;
            
            bool is_rt = (p->priority == 0);
            int& global_free = is_rt ? free_rt_frames : free_user_frames;

            // O processo aloca um novo frame se não tiver chegado no seu limite (working set) 
            // e se ainda houver frames disponíveis na região global correspondente
            if (p_mem.allocated_frames < p->frames && global_free > 0) {
                p_mem.allocated_frames++;
                global_free--;
                p_mem.pages.push_back(page);
            } else {
                // algoritmo LRU no escopo local
                // remove a página menos recentemente usada (frente da lista)
                if (!p_mem.pages.empty()) {
                    p_mem.pages.pop_front();
                }
                // insere a nova pagina no final (mais recentemente usada)
                p_mem.pages.push_back(page);
            }
        }
    }
}

void MemoryManager::terminate_process(Process* p) {
    auto it = memory_table.find(p->pid);
    if (it != memory_table.end()) {
        // devolve os frames alocados para o pool global
        bool is_rt = (p->priority == 0);
        if (is_rt) {
            free_rt_frames += it->second.allocated_frames;
        } else {
            free_user_frames += it->second.allocated_frames;
        }
        memory_table.erase(it);
    }
}