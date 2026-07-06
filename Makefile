# Compilador e flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

# Diretórios
SRC_DIR = src
INC_DIR = include

# Encontra todos os arquivos .cpp na pasta src automaticamente
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Define os arquivos objeto (.o) correspondentes
OBJS = $(SRCS:.cpp=.o)

# Nome do executável final (ficará na raiz do projeto)
TARGET = dispatcher

# Regra principal (o que roda quando você digita apenas 'make')
all: $(TARGET)

# linkar o executável final
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# compilar cada arquivo .cpp em um arquivo .o
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regra de atalho para compilar e rodar o projeto de uma vez
run: $(TARGET)
	./$(TARGET) processes.txt files.txt string.txt

# Regra para limpar os arquivos compilados 
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)
	rm -f $(SRC_DIR)/a.out

# Diz ao make que essas palavras não são arquivos físicos
.PHONY: all clean run