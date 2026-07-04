#include "../include/ResourceManager.hpp"
// #include "../include/QueueManager.hpp" // Descomente se for utilizar
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib> // Para o exit() e EXIT_FAILURE

ResourceManager::ResourceManager(const std::string& file) {
    std::ifstream myFile(file);
    
    if (!myFile.is_open()) {
        std::cerr << "Erro! Não foi possível abrir o arquivo " << file << "\n";
        exit(EXIT_FAILURE);
    }
    
    int count = 0;
    std::string line;
    
    while (std::getline(myFile, line)) {
        // Ignora linhas vazias
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        
        // Linha 0: Quantidade de blocos no disco
        if (count == 0) {
            ss >> this->disk_blocks;
            // Popula o disco
            for(int i = 0; i < disk_blocks; i++) {
            	disk.push_back(0);
            }
        } 
        // Linha 1: Quantidade de segmentos ocupados
        else if (count == 1) {
            ss >> this->occupied_segments;
        } 
        // Linhas 2 até (1 + occupied_segments): Instruções de segmentos ocupados
        else if (count <= 1 + occupied_segments) {
            char nome;
            char virgula;
            int first_block, block_quantity;
            
            if (ss >> nome >> virgula >> first_block >> virgula >> block_quantity) {
                this->occupied_segments_instructions.push_back({nome, first_block, block_quantity});
            }
        } 
        // Restante das linhas: Instruções de processos
        else {
            int process, operation;
            char nome;
            char virgula;
            int block_quantity = 0; // Inicializa com 0 por padrão
            
            // Lê primeiro as 3 variáveis comuns a ambas as operações
            if (ss >> process >> virgula >> operation >> virgula >> nome) {
                if (operation == 0) {
                    // Se for criação (0), precisa ler a quantidade de blocos
                    if (ss >> virgula >>  block_quantity) {
                        this->process_instructions.push_back({process, operation, nome, block_quantity});
                    }
                } 
                else if (operation == 1) {
                    // Se for deleção (1), a quantidade de blocos é 0 (ou irrelevante)
                    this->process_instructions.push_back({process, operation, nome, 0});
                } 
                else {
                    std::cerr << "Operação inválida!\n";
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        count++;
    }

    myFile.close();
}

void ResourceManager::execInstruction(Process* p) {
	for (std::tuple t : p->process_instructions){
		std::cout << std::get<0>(t) << "\n";
	}
}
    
