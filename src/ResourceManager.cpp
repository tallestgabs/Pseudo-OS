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
            	disk.push_back('_');
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

void ResourceManager::execSegmentsInstruction() {
	for(std::tuple t : occupied_segments_instructions) {
		std::cout << std::get<0>(t) << " " << std::get<1>(t) << " " << std::get<2>(t) << "\n";
		char file_name = std::get<0>(t);
		int first_block = std::get<1>(t);
		int quantity = std::get<2>(t);
		bool test = true;
		// Checagem para ver ser o espaço está livre
		for(int i = first_block; i < first_block + quantity; i++) {
			if(bit_map[i]){
				test = false;
        	}
        }
        if(test) {
			// Aloca no disco os arquivos e atualiza o bit map
			for(int i = first_block; i < first_block + quantity; i++) {
				disk[i]	= file_name;
				bit_map.set(i);
			}
		}
	} 	
}

void ResourceManager::execProcessInstruction(Process* p) {
	std::vector<std::tuple<int, int, char, int>> v = p->process_instructions;
	// Verificação se o vector está vazio
	if(v.empty())
		return;
	std::cout << std::get<0>(v[0]) << " " << std::get<1>(v[0]) << " " << std::get<2>(v[0]) << " " << std::get<3>(v[0]) << "\n";
	char file_name = std::get<2>(v[0]);
	int opp_type = std::get<1>(v[0]);
	int necessary_blocks = std::get<3>(v[0]);
	bool test = true;
	int count = 0;
	int accumulator = 0;
	int first_block = 0;
	// Checagem para ver ser há um espaço suficiente para o arquivo
	if(opp_type == 0) { 
		for(int i = 0; i < disk_blocks; i++) {
			if (count == necessary_blocks) {
				test = true;
				break;
			}
			if(bit_map[i]){
				test = false;
				first_block += accumulator;
				accumulator = 0;
				count = 0;
				first_block += 1;
		    }else {
		    	count += 1;
		    	accumulator += 1;
			}
		}
		// Necessário para testar o último bloco
		if (count == necessary_blocks) {
				test = true;
		}

		if(test) {
			// Aloca no disco os arquivos e atualiza o bit map
			for(int i = first_block; i < first_block + necessary_blocks; i++) {
				disk[i]	= file_name;
				bit_map.set(i);
			}
		}
	} else if(opp_type == 1) {
		for(char c : disk) {
			if(c == file_name) {
				test = true;
				bit_map.reset(count);
				disk[count] = '_';
			}
			count += 1;		
		}
	}
	// Deleta a instrução do vector instruções do processo pop.
	(p->process_instructions).erase(p->process_instructions.begin() + 0);
}
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
