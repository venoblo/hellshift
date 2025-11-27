#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include <stdbool.h>

struct Player;

// tamanho do mapa (= tamanho de uma sala)
#define MAP_WIDTH   20
#define MAP_HEIGHT  12

#define SPRITE_SIZE 16
#define TILE_SIZE   32

#define MAP_OFFSET_X 0
#define MAP_OFFSET_Y 0

#define TILE_EMPTY 0

#define TILE_WALL_TOP     1
#define TILE_WALL_BOTTOM  2
#define TILE_WALL_LEFT    3
#define TILE_WALL_RIGHT   4
#define TILE_WALL_TL      5
#define TILE_WALL_TR      6
#define TILE_WALL_BL      7
#define TILE_WALL_BR      8

#define TILE_TRAP 9

#define TILE_DOOR 10
#define TILE_DOOR_CLOSED 11

#define TILE_CHEST_CLOSED  12
#define TILE_CHEST_OPEN     13

#define FLOOR_1 14
#define FLOOR_2 15
#define FLOOR_3 16
#define FLOOR_4 17
#define FLOOR_5 18
#define FLOOR_6 19
#define FLOOR_7 20
#define FLOOR_8 21
#define FLOOR_9 22
#define FLOOR_10 23
#define FLOOR_11 24
#define FLOOR_12 25
#define FLOOR_13 26
#define FLOOR_14 27
#define FLOOR_15 28
#define FLOOR_16 29
#define FLOOR_17 30
#define FLOOR_18 31
#define FLOOR_19 32
#define FLOOR_20 33
#define FLOOR_21 34

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
    int floorLevel;
} Dungeon;

typedef struct Map {
    Dungeon dungeon;
    int tiles[MAP_HEIGHT][MAP_WIDTH];
} Map;


// Funções públicas
void LoadMap(Map *map, const char *fileName);
void DrawMap(Map map);

bool CheckMapCollision(Map map, Vector2 worldPos);
bool CheckTrapInteraction(Map *map, Vector2 worldPos);

// troca de sala ao passar pelas portas
void CheckRoomTransition(Map *map, struct Player *p1, struct Player *p2, int numPlayers);

void GoToNextFloor(Map *map, Vector2 *p1Pos, Vector2 *p2Pos, int numPlayers);

// Minimapa
void DrawMiniMap(Map *map, bool expanded);

// textura do mapa
void LoadMapTextures(void);
void UnloadMapTextures(void);

// cura o player ao abrir bau
bool TryOpenChest(Map *map, struct Player *p);

#endif