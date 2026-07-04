#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <vector>
#include <string>
#include <tuple>
#include "Process.hpp"

class ResourceManager{
	private:
	
	int disk_blocks;
	int occupied_segments;
	std::vector<int> disk;
	std::vector<std::tuple<char, int, int>> occupied_segments_instructions;
	std::vector<std::tuple<int, int, char, int>> process_instructions;
	
	public:
	// Recebe o file "files.txt"e ocupa as variáveis
	ResourceManager(const std::string& file);
	
	// Getters 
	int getDiskBlocks() const { return disk_blocks; }
	
    int getOccupiedSegments() const { return occupied_segments; }
    
    std::vector<int> getDisk() { return disk; }
    
    const std::vector<std::tuple<char, int, int>>& getOccupiedSegmentsInstructions() const { 
    	 return occupied_segments_instructions; 
    }
    const std::vector<std::tuple<int, int, char, int>>& getProcessInstructions() const { 
    	 return process_instructions; 
    }
    
    // Executa a instrução de arquivo do respectivo processo
    void execInstruction(Process* p);
    
};

#endif
