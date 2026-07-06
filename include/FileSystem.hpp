#ifndef FILESYSTEM_HPP 
#define FILESYSTEM_HPP


#include <bitset>
#include <vector>
#include <string>
#include <tuple>
#include <string>
#include "Process.hpp"

class FileSystem{
	private:
	
	int disk_blocks;														// Total de blocos
	int occupied_segments;													// Segmentos ocupados
	std::bitset<1000> bit_map;												// Mapa de bits
	std::vector<std::pair<char, int>> disk;									// Disco: {nome_do_arquivo, pid_do_criador (-1 = nenhum/sistema)}
	std::vector<std::tuple<char, int, int>> occupied_segments_instructions; // Instruções da ocupação dos segmentos
	std::vector<std::tuple<int, int, char, int>> process_instructions;      // Instruções dos processos
	
	public:
	// Recebe o file "files.txt"e ocupa as variáveis
	FileSystem(const std::string& file);
	
	// Getters 
	int getDiskBlocks() const { return disk_blocks; }
	
    int getOccupiedSegments() const { return occupied_segments; }
    
    std::vector<std::pair<char, int>> getDisk() { return disk; }
    
    std::bitset<1000> getBitMap() { return bit_map; }
    
    const std::vector<std::tuple<char, int, int>>& getOccupiedSegmentsInstructions() const { 
    	 return occupied_segments_instructions; 
    }
    
    const std::vector<std::tuple<int, int, char, int>>& getProcessInstructions() const { 
    	 return process_instructions; 
    }
    
    void execSegmentsInstruction();
    
    // Executa a instrução de arquivo do respectivo processo
    void execProcessInstruction(Process* p);
    
    void execAllProcessInstructions(const std::vector<Process*>& processes); 
    
    std::vector<std::string> file_system_log;
    
};

#endif
