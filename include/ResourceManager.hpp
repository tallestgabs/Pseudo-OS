#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "Process.hpp"

class ResourceManager {
    private:
        // Recursos disponíveis no pseudo-SO
        int available_printers = 2;
        int available_scanners = 1;
        int available_modems = 1;
        int available_satas = 2;

    public:

        ResourceManager() = default;

        // só aloca se todos os recursos solicitados estiverem livres
        bool verify_andAllocate(Process* p);

        // Devolve os recursos ao pool do sistema quando o processo termina
        void free_resources(Process* p);
};

#endif