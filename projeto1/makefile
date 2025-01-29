CC = gcc
CFLAGS = -Wall -Wextra -O2 -lpthread
TARGET = decodificador
INPUT_DIR = senhas
OUTPUT_DIR = resultados

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) *.o
	rm -rf $(OUTPUT_DIR)/dec_passwd_*.txt

setup:
	mkdir -p $(INPUT_DIR) $(OUTPUT_DIR)

run: $(TARGET) setup
	./$(TARGET)

.PHONY: all clean run setup