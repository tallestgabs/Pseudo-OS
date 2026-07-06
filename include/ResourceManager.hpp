#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "Process.hpp"

class ResourceManager {
    private:
        // Recursos disponíveis no pseudo-SO (alterados durante a execução)
        int available_printers = max_printers;
        int available_scanners = max_scanners;
        int available_modems = max_modems;
        int available_satas = max_satas;

    public:

        ResourceManager() = default;

        // só aloca se todos os recursos solicitados estiverem livres
        bool verify_andAllocate(Process* p);

        // Devolve os recursos ao pool do sistema quando o processo termina
        void free_resources(Process* p);

        // Valores maximos do recurso (se um processo pedir mais que o maximo é eliminado)
        static const int max_printers = 2;
        static const int max_scanners = 1;
        static const int max_modems = 1;
        static const int max_satas = 2;
};

#endif