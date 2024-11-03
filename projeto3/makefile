# Nome do executável
TARGET = servidor

# Compilador
CC = gcc

# Opções de compilação
CFLAGS = -Wall

# Bibliotecas
LDFLAGS = -lpthread

# Arquivo de entrada
SRC = main.c

# Regra para compilar o programa
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Limpeza dos arquivos objetos e executáveis
clean:
	rm -f $(TARGET)
