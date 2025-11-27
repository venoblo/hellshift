# Nome do executável final
TARGET = hellshift

# Compilador C
CC = gcc

# LISTA DE TODOS OS ARQUIVOS .C
SRCS = main.c player.c monster.c projectile.c map.c save.c
# Flags de compilação
# Adicionado /opt/homebrew para Macs M1/M2/M3
CFLAGS = -Wall -std=c99 -g -I/usr/local/include -I/opt/homebrew/include

# DETECÇÃO DE SISTEMA OPERACIONAL
UNAME_S := $(shell uname -s)

# Configuração do macOS (Darwin)
ifeq ($(UNAME_S),Darwin)
    # --- RPATH PARA NÃO TRAVAR AO ABRIR ---
    LDFLAGS = -L/usr/local/lib -L/opt/homebrew/lib -lraylib \
              -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -lm \
              -Wl,-rpath,/usr/local/lib \
              -Wl,-rpath,/opt/homebrew/lib \
              -Wl,-rpath,/usr/local/opt/raylib/lib
    EXECUTABLE = $(TARGET)

# Configuração do Windows (MINGW)
else ifneq ($(findstring MINGW,$(UNAME_S)),)
    CFLAGS += -I/mingw64/include -DPLATFORM_DESKTOP
    LDFLAGS = -L/mingw64/lib -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows 
    EXECUTABLE = $(TARGET).exe

# Configuração do Linux
else
    LDFLAGS = -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    EXECUTABLE = $(TARGET)
endif

# Converte a lista de .c em .o
OBJS = $(SRCS:.c=.o)

# --- REGRAS DE COMPILAÇÃO ---

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) -o $(EXECUTABLE) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXECUTABLE) $(OBJS)

run: all
	./$(EXECUTABLE)