#include "map.h"
#include "player.h"
#include "monster.h"    
#include "projectile.h"
#include <stdio.h> 

#ifndef MAP_OFFSET_X
#define MAP_OFFSET_X 0
#define MAP_OFFSET_Y 0
#endif

// (Leitura de Arquivo)
void LoadMap(Map *map, const char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        // Se falhar, preenche com 0 para não crashar
        for(int y=0; y<MAP_HEIGHT; y++) 
            for(int x=0; x<MAP_WIDTH; x++) 
                map->tiles[y][x] = 0;
        return;
    }

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
                .x = x * TILE_SIZE + MAP_OFFSET_X, 
                .y = y * TILE_SIZE + MAP_OFFSET_Y,
                .width = TILE_SIZE,
                .height = TILE_SIZE
            };

            if (map.tiles[y][x] == 1) { /* Parede */ }
            else if (map.tiles[y][x] == 2) { // Armadilha
                DrawRectangleRec(tileRec, MAROON);
                DrawCircle(tileRec.x + 20, tileRec.y + 20, 10, RED); // Detalhe
            }
            else { // Chão
                DrawRectangleRec(tileRec, DARKGRAY);
            }
        }
    }
}

// A função mais importante: Checa Colisão
bool CheckMapCollision(Map map, Vector2 worldPos) {

    float localX = worldPos.x - MAP_OFFSET_X;
    float localY = worldPos.y - MAP_OFFSET_Y;
    // 1. Converte a posição do mundo (pixels) para a grade do mapa
    int mapX = (int)(localX / TILE_SIZE); 
    int mapY = (int)(localY / TILE_SIZE);

    // 2. Checa se está dentro dos limites do mapa
    if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
        return true; // Fora do mapa é considerado colisão
    }

    // 3. Retorna se a célula é uma parede (1)
    return (map.tiles[mapY][mapX] == 1);
}

bool CheckTrapInteraction(Map *map, Vector2 worldPos) {
    // 1. Converte posição do mundo para índice da matriz (usando Offsets)
    int mapX = (int)((worldPos.x - MAP_OFFSET_X) / TILE_SIZE);
    int mapY = (int)((worldPos.y - MAP_OFFSET_Y) / TILE_SIZE);

    // 2. Verifica se está dentro dos limites
    if (mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT) {
        
        // 3. Se for uma armadilha (Tipo 2)
        if (map->tiles[mapY][mapX] == 2) {
            map->tiles[mapY][mapX] = 0; // <--- CONSOME A ARMADILHA (Vira chão)
            return true; // Sinaliza que explodiu
        }
    }
    return false;
}