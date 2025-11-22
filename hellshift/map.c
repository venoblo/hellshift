#include "map.h"
#include "monster.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================
   Funções INTERNAS
========================================= */

// Copia a sala atual para map->tiles
static void LoadRoomToMap(Map *map) {
    Room *r = &map->dungeon.rooms[map->dungeon.currentRoom];

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map->tiles[y][x] = r->tiles[y][x];
        }
    }
}

// Encontra sala pelas coordenadas de grid
static int FindRoom(Dungeon *d, int gx, int gy) {
    for (int i = 0; i < d->roomCount; i++) {
        if (d->rooms[i].gridX == gx && d->rooms[i].gridY == gy)
            return i;
    }
    return -1;
}

// Cria paredes + chão + portas
static void BuildRoom(Room *r) {

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1)
                r->tiles[y][x] = TILE_WALL;
            else
                r->tiles[y][x] = 0;
        }
    }

    int midX = MAP_WIDTH / 2;
    int midY = MAP_HEIGHT / 2;

    if (r->doorUp)    r->tiles[0][midX] = TILE_DOOR;
    if (r->doorDown)  r->tiles[MAP_HEIGHT - 1][midX] = TILE_DOOR;
    if (r->doorLeft)  r->tiles[midY][0] = TILE_DOOR;
    if (r->doorRight) r->tiles[midY][MAP_WIDTH - 1] = TILE_DOOR;

    // Armadilhas aleatórias
    if (r->type == ROOM_NORMAL) {
        int traps = GetRandomValue(1, 3);
        for (int i = 0; i < traps; i++) {
            int rx = GetRandomValue(2, MAP_WIDTH - 3);
            int ry = GetRandomValue(2, MAP_HEIGHT - 3);

            if (r->tiles[ry][rx] == 0)
                r->tiles[ry][rx] = TILE_TRAP;
        }
    }
}


// Gera dungeon ramificada
static void GenerateDungeon(Dungeon *d) {

    d->roomCount = 0;

    // Sala inicial
    Room *start = &d->rooms[0];
    memset(start, 0, sizeof(Room));
    start->gridX = 0;
    start->gridY = 0;
    start->type = ROOM_START;
    start->cleared = true; // nunca tem inimigos
    d->roomCount = 1;

    int maxRooms = 12;

    for (int i = 0; i < maxRooms; i++) {

        int baseIndex = GetRandomValue(0, d->roomCount - 1);
        Room *base = &d->rooms[baseIndex];

        int dir = GetRandomValue(0, 3);
        int nx = base->gridX;
        int ny = base->gridY;

        if (dir == 0) ny--;
        if (dir == 1) ny++;
        if (dir == 2) nx--;
        if (dir == 3) nx++;

        if (FindRoom(d, nx, ny) != -1) continue;
        if (d->roomCount >= MAX_ROOMS) break;

        Room *newRoom = &d->rooms[d->roomCount];
        memset(newRoom, 0, sizeof(Room));

        newRoom->gridX = nx;
        newRoom->gridY = ny;
        newRoom->cleared = false;

        int roll = GetRandomValue(0, 10);

        if (roll == 0)
            newRoom->type = ROOM_TREASURE;
        else
            newRoom->type = ROOM_NORMAL;

        if (dir == 0) { base->doorUp    = true; newRoom->doorDown  = true; }
        if (dir == 1) { base->doorDown  = true; newRoom->doorUp    = true; }
        if (dir == 2) { base->doorLeft  = true; newRoom->doorRight = true; }
        if (dir == 3) { base->doorRight = true; newRoom->doorLeft  = true; }

        d->roomCount++;
    }

    // Última sala vira boss
    d->rooms[d->roomCount - 1].type = ROOM_BOSS;
    d->rooms[d->roomCount - 1].cleared = false;

    for (int i = 0; i < d->roomCount; i++)
        BuildRoom(&d->rooms[i]);

    d->currentRoom = 0;
}


/* =========================================
   Funções PÚBLICAS
========================================= */

void LoadMap(Map *map, const char *fileName) {
    (void)fileName;
    GenerateDungeon(&map->dungeon);
    LoadRoomToMap(map);
}

void DrawMap(Map map) {

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {

            Rectangle tileRec = {
                .x = x * TILE_SIZE + MAP_OFFSET_X,
                .y = y * TILE_SIZE + MAP_OFFSET_Y,
                .width = TILE_SIZE,
                .height = TILE_SIZE
            };

            int t = map.tiles[y][x];

            if (t == TILE_WALL)
                DrawRectangleRec(tileRec, GRAY);

            else if (t == TILE_TRAP) {
                DrawRectangleRec(tileRec, MAROON);
                DrawCircle(tileRec.x + 20, tileRec.y + 20, 10, RED);
            }

            else if (t == TILE_DOOR)
                DrawRectangleRec(tileRec, GOLD);

            else
                DrawRectangleRec(tileRec, DARKGRAY);
        }
    }
}

bool CheckMapCollision(Map map, Vector2 worldPos) {

    float localX = worldPos.x - MAP_OFFSET_X;
    float localY = worldPos.y - MAP_OFFSET_Y;

    int mapX = (int)(localX / TILE_SIZE);
    int mapY = (int)(localY / TILE_SIZE);

    if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT)
        return true;

    return (map.tiles[mapY][mapX] == TILE_WALL);
}

bool CheckTrapInteraction(Map *map, Vector2 worldPos) {

    int mapX = (int)((worldPos.x - MAP_OFFSET_X) / TILE_SIZE);
    int mapY = (int)((worldPos.y - MAP_OFFSET_Y) / TILE_SIZE);

    if (mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT) {

        if (map->tiles[mapY][mapX] == TILE_TRAP) {
            map->tiles[mapY][mapX] = 0;
            map->dungeon.rooms[map->dungeon.currentRoom].tiles[mapY][mapX] = 0;
            return true;
        }
    }

    return false;
}


// MUDA DE SALA + SPAWN DE MONSTROS
void CheckRoomTransition(Map *map, Vector2 *p) {

    Dungeon *d = &map->dungeon;
    Room *r = &d->rooms[d->currentRoom];

    int midX = (MAP_WIDTH * TILE_SIZE) / 2;
    int midY = (MAP_HEIGHT * TILE_SIZE) / 2;

    int rightLimit = MAP_WIDTH * TILE_SIZE - 10;
    int leftLimit = 10;
    int downLimit = MAP_HEIGHT * TILE_SIZE - 10;
    int upLimit = 10;

    int nextIndex = -1;

    if (r->doorRight && p->x > rightLimit)
        nextIndex = FindRoom(d, r->gridX + 1, r->gridY);

    else if (r->doorLeft && p->x < leftLimit)
        nextIndex = FindRoom(d, r->gridX - 1, r->gridY);

    else if (r->doorDown && p->y > downLimit)
        nextIndex = FindRoom(d, r->gridX, r->gridY + 1);

    else if (r->doorUp && p->y < upLimit)
        nextIndex = FindRoom(d, r->gridX, r->gridY - 1);

    if (nextIndex == -1)
        return;

    d->currentRoom = nextIndex;
    LoadRoomToMap(map);

    Room *newRoom = &d->rooms[d->currentRoom];

    // Teleporte seguro
    if (p->x > rightLimit) { p->x = TILE_SIZE * 2; p->y = midY; }
    else if (p->x < leftLimit) { p->x = (MAP_WIDTH - 3) * TILE_SIZE; p->y = midY; }
    else if (p->y > downLimit) { p->x = midX; p->y = TILE_SIZE * 2; }
    else if (p->y < upLimit) { p->x = midX; p->y = (MAP_HEIGHT - 3) * TILE_SIZE; }

    // ================================
    // SPAWN DE MONSTROS POR SALA
    // ================================
    if (!newRoom->cleared) {

        if (newRoom->type == ROOM_NORMAL) {
            int qtd = GetRandomValue(3, 6);

            for (int i = 0; i < qtd; i++) {
                Vector2 pos = {
                    GetRandomValue(80, 700),
                    GetRandomValue(80, 400)
                };

                SpawnMonster(pos, MONSTER_SKELETON);
            }
        }

        else if (newRoom->type == ROOM_BOSS) {
            SpawnMonster((Vector2){360, 200}, MONSTER_SHADOW_MELEE);
            SpawnMonster((Vector2){400, 200}, MONSTER_SHADOW_SPELL);
        }

        newRoom->cleared = true;
    }
}