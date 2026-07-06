# Simulador de Sistema Operacional (Pseudo-SO)

![image](https://github.com/user-attachments/assets/dfad9149-cb02-4821-8b15-1ed1b413db16)

## Universidade de Brasília

Este repositório contém o código-fonte do Trabalho Prático da disciplina de **Fundamentos de Sistemas Operacionais (FSO)**, desenvolvido na **Universidade de Brasília (UnB)**.

O projeto consiste na implementação de um simulador de Sistema Operacional multiprogramado. Ele foi projetado para administrar o ciclo de vida das tarefas em um ambiente de concorrência, lidando com CPU, Memória Virtual, Dispositivos de E/S e Sistema de Arquivos.

## 🚀 Arquitetura e Módulos

O motor do simulador foi dividido em quatro frentes principais, seguindo as melhores práticas de Engenharia de Software:

- **Gerenciamento de Processos (Dispatcher):** Escalonador baseado em Múltiplas Filas de Prioridade (0 para Tempo Real, 1 a 3 para Usuário). Conta com preempção por _quantum_ e implementação do mecanismo de **Fila com Realimentação e Aging**, prevenindo completamente a inanição (_starvation_).
- **Gerenciamento de Recursos (I/O):** Sistema de liberação e bloqueio de hardware (Impressoras, Scanners, Modems, SATA). Utiliza a estratégia de alocação atômica ("Tudo ou Nada") no momento do despacho, garantindo a prevenção matemática contra **Deadlocks**.
- **Gerenciamento de Memória Virtual:** Simulador de paginação de memória utilizando o algoritmo **LRU (Least Recently Used)** para substituição de quadros, acompanhado do rastreamento de Faltas de Página (_Page Faults_).
- **Sistema de Arquivos:** Simulação de um disco virtual utilizando a estratégia de alocação contígua **First-Fit**, controlando a criação, deleção e fragmentação dos blocos.

## 🛠️ Tecnologias Utilizadas

- **Linguagem:** C++ (Padrão C++17)
- **Build System:** GNU Make
- **Ambiente de Desenvolvimento Recomendado:** Linux
- **Dependências Extras:** Nenhuma. O projeto utiliza exclusivamente a biblioteca padrão do C++ (STL).

## 📂 Estrutura do Projeto

```text
.
├── include/                 # Arquivos de cabeçalho (.hpp)
│   ├── FileSystem.hpp
│   ├── MemoryManager.hpp
│   ├── Process.hpp
│   ├── QueueManager.hpp
│   └── ResourceManager.hpp
├── src/                     # Código-fonte (.cpp)
│   ├── FileSystem.cpp
│   ├── main.cpp
│   ├── MemoryManager.cpp
│   ├── QueueManager.cpp
│   └── ResourceManager.cpp
├── Makefile                 # Automação de compilação
├── processes.txt            # Input: Processos
├── files.txt                # Input: Disco
├── string.txt               # Input: Memória
└── README.md
```

## ⚙️ Como Compilar e Executar

Graças ao Makefile configurado na raiz do projeto, a compilação e execução foram abstraídas para comandos simples no terminal.

### 1. Para compilar o projeto (gera o executável dispatcher):

```bash
make
```

### 2. Para compilar e executar automaticamente com os arquivos de teste padrão:

```bash
make run
```

### 3. Para executar manualmente (caso queira passar outros arquivos):

```bash
./dispatcher processes.txt files.txt string.txt
```

### 4. Para limpar os arquivos compilados:

```bashBash
make clean
```

# 👥 Autores

**Gabriel de Castro Dias**

**Erick Hideki Taira**

**Davi de Araujo Garcez Bueno**
