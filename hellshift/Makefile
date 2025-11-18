# Makefile para o Projeto Hellshift (Mac/Linux)

# Nome do executável final
TARGET = hellshift

# Compilador C
CC = gcc

# 1. CORREÇÃO: Lista de TODOS os seus arquivos .c COM O CAMINHO
SRCS = hellshift/main.c hellshift/player.c hellshift/monster.c hellshift/projectile.c hellshift/map.c

# 2. CORREÇÃO CRÍTICA: Manter o prefixo 'hellshift/' nos objetos
# Isso resulta em: hellshift/main.o hellshift/player.o ...
OBJS = $(patsubst hellshift/%.c, hellshift/%.o, $(SRCS))

# Flags de compilação
CFLAGS = -Wall -std=c99 -g -I/usr/local/include -I/opt/homebrew/include

# Bibliotecas e Flags de Linkagem para Mac (Darwin)
LDFLAGS = -L/usr/local/lib -L/opt/homebrew/lib -lraylib -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -lm


# ---------------- REGRAS PRINCIPAIS ----------------

# Regra padrão: Constrói o executável a partir dos arquivos .o
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Regra para converter .c em .o (esta é a regra que falhava antes)
# Agora ele sabe como lidar com o prefixo hellshift/
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 3. Regra para rodar: make run
run: all
	./$(TARGET)

# 4. Regra para limpar: make clean
clean:
	rm -f $(TARGET) $(OBJS)