# O nome do seu executável final (sem extensão)
TARGET = game

# O compilador C
CC = gcc

# Lista dos seus arquivos-fonte .c
SRCS = main.c

# Flags de compilação (avisos, padrão C99, debug)
CFLAGS = -Wall -std=c99 -g

# --- DETECÇÃO DE SISTEMA OPERACIONAL ---
UNAME_S := $(shell uname -s)

# --- Configuração do macOS (Darwin) ---
ifeq ($(UNAME_S),Darwin)
	CFLAGS += -I/usr/local/include
	LDFLAGS = -L/usr/local/lib -lraylib -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
	EXECUTABLE = $(TARGET)

# --- Configuração do Windows (MINGW) ---
else ifneq ($(findstring MINGW,$(UNAME_S)),)
	CFLAGS += -I/mingw64/include -DPLATFORM_DESKTOP
	LDFLAGS = -L/mingw64/lib -lraylib -lopengl32 -lgdi32 -lwinmm
	LDFLAGS += -mwindows 
	EXECUTABLE = $(TARGET).exe

# --- Configuração do Linux (padrão) ---
else
	# Agora aponta para /usr/local, assim como o Mac!
	CFLAGS += -I/usr/local/include
	LDFLAGS = -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
	EXECUTABLE = $(TARGET)
endif
# ----------------------------------------

# Converte a lista de .c em .o (arquivos-objeto)
OBJS = $(SRCS:.c=.o)


# --- REGRAS DE COMPILAÇÃO ---
# Lembre-se: As linhas de comando devem começar com um TAB

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) -o $(EXECUTABLE) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	# -f ignora erros se os arquivos não existirem
	rm -f $(EXECUTABLE) $(OBJS)

run: all
	./$(EXECUTABLE)