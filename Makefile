# O nome do seu executável final
TARGET = game

# O compilador C
CC = gcc

# Lista dos seus arquivos-fonte .c (por enquanto, só main.c)
SRCS = main.c

# Flags de compilação (avisos, padrão C99, debug)
CFLAGS = -Wall -std=c99 -g

# --- DETECÇÃO DE SISTEMA OPERACIONAL ---
UNAME_S := $(shell uname -s)

# --- Configuração do macOS (Darwin) ---
ifeq ($(UNAME_S),Darwin)
    # Encontra onde o Homebrew (brew) instalou as coisas
    BREW_PREFIX := $(shell brew --prefix)

    # Diz ao compilador onde achar o "raylib.h"
    CFLAGS += -I$(BREM_PREFIX)/include

    # Diz ao "linker" quais bibliotecas usar e onde achá-las
    # ATENÇÃO: Seu Mac é mais antigo (macOS 12) e Intel,
    # então o Homebrew instalou em /usr/local.
    # Vamos forçar esse caminho para garantir.
    LDFLAGS = -L/usr/local/lib -lraylib -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL

# --- Configuração do Linux ---
else
    # Caminhos padrão do Linux
    CFLAGS += -I/usr/include
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif
# ----------------------------------------

# Converte a lista de .c em .o (arquivos-objeto)
OBJS = $(SRCS:.c=.o)


# --- REGRAS DE COMPILAÇÃO ---

# Regra 'all': O que fazer quando você digita só 'make'
all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Regra de Compilação: Como transformar qualquer .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra 'clean': Limpa os arquivos compilados
clean:
	rm -f $(TARGET) $(OBJS)

# Regra 'run': Compila E Roda o jogo
run: all
	./$(TARGET)