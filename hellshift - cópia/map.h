#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include <stdbool.h>

struct Player;

// tamanho do mapa (= tamanho de UMA SALA)
#define MAP_WIDTH   20
#define MAP_HEIGHT  12
#define TILE_SIZE   40

#define MAP_OFFSET_X 0
#define MAP_OFFSET_Y 0
#define TILE_TRAP 2
#define TILE_WALL 1
#define TILE_DOOR 3

#define MAX_ROOMS 20   // quantidade máxima de salas na dungeon

// Tipo de sala
typedef enum {
    ROOM_START,
    ROOM_NORMAL,
    ROOM_BOSS,
    ROOM_TREASURE
} RoomType;

// Cada sala individual
typedef struct {
    int gridX;
    int gridY;
    RoomType type;

    bool doorUp;
    bool doorDown;
    bool doorLeft;
    bool doorRight;
    bool portalActive;
    bool cleared;
    bool visited;
    bool discovered;

    int tiles[MAP_HEIGHT][MAP_WIDTH];
} Room;

// A dungeon é uma lista de salas
typedef struct {
    Room rooms[MAX_ROOMS];
    int roomCount;
    int currentRoom;
    int floorLevel;   // andar atual (1 - 7)
} Dungeon;

// A Struct do Map agora CONTÉM uma dungeon
typedef struct Map {
    Dungeon dungeon;
    int tiles[MAP_HEIGHT][MAP_WIDTH]; // espelho da sala atual
} Map;


// Funções públicas
void LoadMap(Map *map, const char *fileName); // AGORA GERA A DUNGEON
void DrawMap(Map map);

bool CheckMapCollision(Map map, Vector2 worldPos);
bool CheckTrapInteraction(Map *map, Vector2 worldPos);

// troca de sala ao passar pelas portas
void CheckRoomTransition(Map *map, struct Player *p1, struct Player *p2, int numPlayers);

void GoToNextFloor(Map *map, Vector2 *p1Pos, Vector2 *p2Pos, int numPlayers);

// Minimapa
void DrawMiniMap(Map *map, bool expanded);

#endif