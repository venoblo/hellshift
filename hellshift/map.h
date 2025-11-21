#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include <stdbool.h>

// tamanho mapa
#define MAP_WIDTH 20
#define MAP_HEIGHT 12 
#define TILE_SIZE 40// tamanho de cada bloco em pixels

#define MAP_OFFSET_X 0
#define MAP_OFFSET_Y 0
#define TILE_TRAP 2

// A Struct do Mapa
typedef struct Map {
    
    int tiles[MAP_HEIGHT][MAP_WIDTH];
} Map;

// Funções públicas
void LoadMap(Map *map, const char *fileName); // lê de Arquivo
void DrawMap(Map map);
bool CheckMapCollision(Map map, Vector2 worldPos); // checa colisão
bool CheckTrapInteraction(Map *map, Vector2 worldPos); //trap

#endif

