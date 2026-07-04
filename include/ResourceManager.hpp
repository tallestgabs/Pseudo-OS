#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <vector>
#include <string>
#include <tuple>

class ResourceManager{
	private:
	
	int disk_blocks;
	int occupied_segments;
	std::vector<std::tuple<char, int, int>> occupied_segments_instructions;
	std::vector<std::tuple<int, int, char, int>> process_instructions;
	
	public:
	// Recebe o file "files.txt"e ocupa as variáveis
	ResourceManager(const std::string& file);
	
	int getDiskBlocks() const { return disk_blocks; }
	
    int getOccupiedSegments() const { return occupied_segments; }
        
    const std::vector<std::tuple<char, int, int>>& getOccupiedSegmentsInstructions() const { 
    	 return occupied_segments_instructions; 
    }
    const std::vector<std::tuple<int, int, char, int>>& getProcessInstructions() const { 
    	 return process_instructions; 
    }
};

#endif
