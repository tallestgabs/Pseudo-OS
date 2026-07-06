#include "../include/ResourceManager.hpp"


bool ResourceManager::verify_andAllocate(Process* p){
    // processos em tempo real nao usam E/S
    if(p->priority == 0) return true;

    // verifica se o sistema possui os recursos solicitados neste milissegundo
    if( available_printers >= p->printer_req &&
        available_scanners >= p->scanner_req &&
        available_modems >= p->modem_req &&
        available_satas >= p->sata_req){
        
            // aloca atomicamente 
            available_printers -= p->printer_req;
            available_scanners -= p->scanner_req;
            available_modems -= p->modem_req;
            available_satas -= p->sata_req;

            return true;    // alocacao bem sucedida
        }
    
    return false;   // se o if falhar os recursos sao insuficientes

}


void ResourceManager::free_resources(Process* p){

    // tempo real nao faz nada, so retorna
    if(p->priority == 0) return;

    // devolve os recursos alocados
    available_printers += p->printer_req;
    available_scanners += p->scanner_req;
    available_modems += p->modem_req;
    available_satas += p->sata_req;
}