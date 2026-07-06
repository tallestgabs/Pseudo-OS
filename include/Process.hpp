#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <vector>

struct Process{
    // metadados 
    int pid;                                                           // inicia em zero
    int page_faults = 0;
    int pc = 0;                                                        // program counter (contagem de instrucoes)
    int waiting_time;                                                  // quantos ciclos o processo ficou parado na fila
    bool has_resources = false;                                        // processos comecam sem nenhum recurso alocado
    std::vector<int> references;                                       // sequencia de paginas lidas no string.txt
    std::vector<std::tuple<int, int, char, int>> process_instructions; // instruções de arquivo de cada processo

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
