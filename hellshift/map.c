#include "map.h"
#include <stdio.h> // Para FILE, fopen, fscanf, fclose

// (Leitura de Arquivo)
void LoadMap(Map *map, const char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        TraceLog(LOG_ERROR, "Não foi possível abrir o arquivo do mapa: %s", fileName);
        return;
    }

    // Loop duplo para preencher a Matriz
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            fscanf(file, "%d", &map->tiles[y][x]);
        }
    }

    fclose(file);
}

// Desenha o mapa
void DrawMap(Map map) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            
            Rectangle tileRec = {
                .x = x * TILE_SIZE,
                .y = y * TILE_SIZE,
                .width = TILE_SIZE,
                .height = TILE_SIZE
            };

            if (map.tiles[y][x] == 1) { // Parede
                DrawRectangleRec(tileRec, GRAY);
            } else { // Chão
                DrawRectangleRec(tileRec, DARKGRAY);
            }
        }
    }
}

// A função mais importante: Checa Colisão
bool CheckMapCollision(Map map, Vector2 worldPos) {
    // 1. Converte a posição do mundo (pixels) para a grade do mapa
    int mapX = (int)(worldPos.x / TILE_SIZE);
    int mapY = (int)(worldPos.y / TILE_SIZE);

    // 2. Checa se está dentro dos limites do mapa
    if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
        return true; // Fora do mapa é considerado colisão
    }

    // 3. Retorna se a célula é uma parede (1)
    return (map.tiles[mapY][mapX] == 1);
}