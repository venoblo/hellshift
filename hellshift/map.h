#ifndef MAP_H
#define MAP_H

#include "raylib.h"

// tamanho mapa
#define MAP_WIDTH 12
#define MAP_HEIGHT 8
#define TILE_SIZE 40 // tamanho de cada bloco em pixels

// A Struct do Mapa
typedef struct Map {
    
    int tiles[MAP_HEIGHT][MAP_WIDTH];
} Map;

// Funções públicas
void LoadMap(Map *map, const char *fileName); // lê de Arquivo
void DrawMap(Map map);
bool CheckMapCollision(Map map, Vector2 worldPos); // checa colisão

#endif