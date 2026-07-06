#include "../include/FileSystem.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib> // Para o exit() e EXIT_FAILURE

// Monta a lista de blocos alocados no formato "0, 1 e 2" para as mensagens de log
static std::string format_block_list(int first_block, int quantity) {
    std::string result;
    for (int i = 0; i < quantity; i++) {
        int block = first_block + i;
        if (i == 0) {
            result += std::to_string(block);
        } else if (i == quantity - 1) {
            result += " e " + std::to_string(block);
        } else {
            result += ", " + std::to_string(block);
        }
    }
    return result;
}

FileSystem::FileSystem(const std::string& file) {
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
            // Popula o disco (bloco livre = {'_', -1}, sem processo criador)
            for(int i = 0; i < disk_blocks; i++) {
            	disk.push_back({'_', -1});
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

void FileSystem::execSegmentsInstruction() {
	for(std::tuple t : occupied_segments_instructions) {
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
			// pid = -1 pois esses arquivos vêm da ocupação inicial dos segmentos,
			// não foram criados por nenhum processo (logo só tempo real pode excluí-los)
			for(int i = first_block; i < first_block + quantity; i++) {
				disk[i]	= {file_name, -1};
				bit_map.set(i);
			}
		}
	} 	
}

// Executa TODAS as instruções de arquivo dos processos, de uma vez, na ordem
// original do arquivo de recursos, e monta o log "Sistema de arquivos".
// Antes essas operações eram executadas silenciosamente, uma por tick de CPU,
// dentro de execProcessInstruction(Process*). Agora elas rodam aqui, em bloco,
// e esvaziam o process_instructions de cada Process para que as chamadas
// remanescentes dentro do loop da CPU virem no-op (não há mais nada pra popar).
void FileSystem::execAllProcessInstructions(const std::vector<Process*>& processes) {
    int operation_number = 1;

    for (std::tuple<int, int, char, int> t : this->process_instructions) {
        int pid = std::get<0>(t);
        int operation = std::get<1>(t);
        char file_name = std::get<2>(t);
        int block_quantity = std::get<3>(t);

        // Processo referenciado não existe
        if (pid < 0 || pid >= (int)processes.size()) {
            this->file_system_log.push_back(
                "\tOperação " + std::to_string(operation_number) + " => Falha\n" +
                "\tO processo " + std::to_string(pid) + " não existe.");
            operation_number++;
            continue;
        }

        if (operation == 0) {
            // Criação de arquivo (mesma lógica de busca de espaço livre já usada)
            bool test = true;
            int count = 0;
            int accumulator = 0;
            int first_block = 0;

            for (int i = 0; i < disk_blocks; i++) {
                if (count == block_quantity) {
                    test = true;
                    break;
                }
                if (bit_map[i]) {
                    test = false;
                    first_block += accumulator;
                    accumulator = 0;
                    count = 0;
                    first_block += 1;
                } else {
                    count += 1;
                    accumulator += 1;
                }
            }
            if (count == block_quantity) {
                test = true;
            }

            if (test) {
                // pid do processo criador fica registrado em cada bloco alocado
                for (int i = first_block; i < first_block + block_quantity; i++) {
                    disk[i] = {file_name, pid};
                    bit_map.set(i);
                }
                this->file_system_log.push_back(
                    "\tOperação " + std::to_string(operation_number) + " => Sucesso\n" +
                    "\tO processo " + std::to_string(pid) + " criou o arquivo " + file_name +
                    " (blocos " + format_block_list(first_block, block_quantity) + ").");
            } else {
                this->file_system_log.push_back(
                    "\tOperação " + std::to_string(operation_number) + " => Falha\n" +
                    "\tO processo " + std::to_string(pid) + " não pode criar o arquivo " +
                    file_name + " (falta de espaço).");
            }
        } else if (operation == 1) {
            // Deleção de arquivo
            bool found = false;
            int owner_pid = -1;

            // Primeiro descobre se o arquivo existe e quem o criou
            for (int i = 0; i < disk_blocks; i++) {
                if (disk[i].first == file_name) {
                    found = true;
                    owner_pid = disk[i].second;
                    break;
                }
            }

            // Processo de tempo real (priority == 0) pode excluir qualquer arquivo.
            // Processo de usuário só pode excluir arquivo que ele mesmo criou.
            bool is_real_time = (processes[pid] != nullptr && processes[pid]->priority == 0);
            bool has_permission = is_real_time || (owner_pid == pid);

            if (found && has_permission) {
                for (int i = 0; i < disk_blocks; i++) {
                    if (disk[i].first == file_name) {
                        bit_map.reset(i);
                        disk[i] = {'_', -1};
                    }
                }
                this->file_system_log.push_back(
                    "\tOperação " + std::to_string(operation_number) + " => Sucesso\n" +
                    "\tO processo " + std::to_string(pid) + " deletou o arquivo " + file_name + ".");
            } else if (!found) {
                this->file_system_log.push_back(
                    "\tOperação " + std::to_string(operation_number) + " => Falha\n" +
                    "\tO processo " + std::to_string(pid) + " não pode deletar o arquivo " +
                    file_name + " porque ele não existe.");
            } else {
                // Arquivo existe, mas o processo não tem permissão para excluí-lo
                this->file_system_log.push_back(
                    "\tOperação " + std::to_string(operation_number) + " => Falha\n" +
                    "\tO processo " + std::to_string(pid) + " não pode deletar o arquivo " +
                    file_name + " porque não foi ele quem o criou.");
            }
        }

        operation_number++;
    }

    // As instruções já foram todas executadas em bloco acima; esvazia os
    // vetores por processo para que o popping por tick de CPU vire no-op.
    for (Process* p : processes) {
        if (p != nullptr)
            p->process_instructions.clear();
    }
}

void FileSystem::execProcessInstruction(Process* p) {
	std::vector<std::tuple<int, int, char, int>> v = p->process_instructions;
	// Verificação se o vector está vazio
	if(v.empty())
		return;
	//std::cout << std::get<0>(v[0]) << " " << std::get<1>(v[0]) << " " << std::get<2>(v[0]) << " " << std::get<3>(v[0]) << "\n";
	int process = std::get<0>(v[0]);
	char file_name = std::get<2>(v[0]);
	int opp_type = std::get<1>(v[0]);
	int necessary_blocks = std::get<3>(v[0]);
	bool test = true;
	int count = 0;
	int accumulator = 0;
	int first_block = 0;
	int operation_count = 1;
	
	if(opp_type == 0) {
		// Checagem para ver ser há um espaço suficiente para o arquivo 
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
				disk[i]	= {file_name, process};
				bit_map.set(i);
			}
		}
	} else if(opp_type == 1) {
		// Nota: esta função legada não tem acesso à lista de processos para
		// checar priority, então a checagem de "usuário só apaga o próprio
		// arquivo" fica concentrada em execAllProcessInstructions, que é o
		// caminho realmente usado hoje.
		for(const std::pair<char,int>& b : disk) {
			if(b.first == file_name) {
				test = true;
				bit_map.reset(count);
				disk[count] = {'_', -1};
			}
			count += 1;		
		}
	}
	operation_count += 1;
	// Deleta a instrução do vector instruções do processo pop.
	(p->process_instructions).erase(p->process_instructions.begin() + 0);
}
