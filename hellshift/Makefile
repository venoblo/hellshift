# Makefile - Versão FINAL, rodando DENTRO da pasta hellshift/

# Nome do executável final (Para evitar conflito com o nome da pasta)
TARGET = hellshift_exec

# Compilador C
CC = gcc

# 1. CORREÇÃO CRÍTICA: Arquivos de fonte estão no diretório atual (sem prefixo)
SRCS = main.c player.c monster.c projectile.c map.c

# Converte a lista de .c em .o (arquivos-objeto)
OBJS = $(SRCS:.c=.o)

# Flags de compilação
CFLAGS = -Wall -std=c99 -g -I/usr/local/include
# Bibliotecas e Flags de Linkagem para Mac (Darwin)
LDFLAGS = -L/usr/local/lib -L/opt/homebrew/lib -lraylib \
        -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -lm \
        -Wl,-rpath,/usr/local/lib \
        -Wl,-rpath,/opt/homebrew/lib \
        -Wl,-rpath,/usr/local/opt/raylib/lib
		

# ---------------- REGRAS PRINCIPAIS ----------------

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Regra para converter .c em .o (funciona agora pois o prefixo sumiu)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para rodar: make run
run: all
	./$(TARGET)

# Regra para limpar: make clean
clean:
	rm -f $(TARGET) $(OBJS)