#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <vector>

struct Process{
    // metadados 
    int pid;  // inicia em zero
    int page_faults = 0;
    int pc = 0;    // program counter (contagem de instrucoes)
    std::vector<int> references; // sequencia de paginas lidas no string.txt

    // dados do process.txt
    int init_time;
    int priority;
    int cpu_time;
    int frames;
    int printer_req;
    int scanner_req;
    int modem_req;
    int sata_req;
};

#endif