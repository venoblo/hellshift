#include "map.h"
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
            if (x == 0 || y == 0 || x == MAP_WIDTH-1 || y == MAP_HEIGHT-1)
                r->tiles[y][x] = TILE_WALL;
            else
                r->tiles[y][x] = 0;
        }
    }

    int midX = MAP_WIDTH / 2;
    int midY = MAP_HEIGHT / 2;

    if (r->doorUp)    r->tiles[0][midX] = TILE_DOOR;
    if (r->doorDown)  r->tiles[MAP_HEIGHT-1][midX] = TILE_DOOR;
    if (r->doorLeft)  r->tiles[midY][0] = TILE_DOOR;
    if (r->doorRight) r->tiles[midY][MAP_WIDTH-1] = TILE_DOOR;

    // Armadilhas aleatórias
    if (r->type == ROOM_NORMAL) {
        for (int i = 0; i < 3; i++) {
            int rx = GetRandomValue(2, MAP_WIDTH-3);
            int ry = GetRandomValue(2, MAP_HEIGHT-3);
            r->tiles[ry][rx] = TILE_TRAP;
        }
    }
}

// Gera dungeon conectada
static void GenerateDungeon(Dungeon *d) {

    d->roomCount = 0;

    // Sala inicial
    Room *start = &d->rooms[0];
    memset(start, 0, sizeof(Room));
    start->gridX = 0;
    start->gridY = 0;
    start->type = ROOM_START;

    d->roomCount = 1;
    int cx = 0, cy = 0;
    int currentIndex = 0;

    for (int i = 1; i < 8; i++) {
        int dir = GetRandomValue(0,3);
        int nx = cx, ny = cy;

        if (dir == 0) ny--;
        if (dir == 1) ny++;
        if (dir == 2) nx--;
        if (dir == 3) nx++;

        if (FindRoom(d, nx, ny) != -1) continue;

        Room *newRoom = &d->rooms[d->roomCount];
        memset(newRoom, 0, sizeof(Room));

        newRoom->gridX = nx;
        newRoom->gridY = ny;
        newRoom->type = (i == 7) ? ROOM_BOSS : ROOM_NORMAL;

        Room *cur = &d->rooms[currentIndex];

        if (nx > cx) { cur->doorRight = true; newRoom->doorLeft = true; }
        if (nx < cx) { cur->doorLeft  = true; newRoom->doorRight = true; }
        if (ny > cy) { cur->doorDown  = true; newRoom->doorUp = true; }
        if (ny < cy) { cur->doorUp    = true; newRoom->doorDown = true; }

        cx = nx;
        cy = ny;
        currentIndex = d->roomCount;
        d->roomCount++;
    }

    for (int i = 0; i < d->roomCount; i++)
        BuildRoom(&d->rooms[i]);

    d->currentRoom = 0;
}

/* =========================================
   Funções PÚBLICAS
========================================= */

void LoadMap(Map *map, const char *fileName) {
    (void)fileName; // não usa mais arquivo
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

            // Atualiza também a sala REAL
            map->dungeon.rooms[map->dungeon.currentRoom].tiles[mapY][mapX] = 0;

            return true;
        }
    }
    return false;
}


// MUDA DE SALA AO PASSAR NA PORTA
void CheckRoomTransition(Map *map, Vector2 *p) {

    Dungeon *d = &map->dungeon;
    Room *r = &d->rooms[d->currentRoom];

    int midX = MAP_WIDTH * TILE_SIZE / 2;
    int midY = MAP_HEIGHT * TILE_SIZE / 2;

    // ---- DIREITA ----
    if (r->doorRight && p->x > (MAP_WIDTH * TILE_SIZE) - (TILE_SIZE/2)) {
        int idx = FindRoom(d, r->gridX + 1, r->gridY);
        if (idx != -1) {
            d->currentRoom = idx;
            LoadRoomToMap(map);

            // Joga para dentro da nova sala (ESQUERDA)
            p->x = TILE_SIZE * 2.5f;
            p->y = midY;
        }
    }

    // ---- ESQUERDA ----
    else if (r->doorLeft && p->x < TILE_SIZE/2) {
        int idx = FindRoom(d, r->gridX - 1, r->gridY);
        if (idx != -1) {
            d->currentRoom = idx;
            LoadRoomToMap(map);

            // Joga para dentro da nova sala (DIREITA)
            p->x = (MAP_WIDTH - 3.5f) * TILE_SIZE;
            p->y = midY;
        }
    }

    // ---- BAIXO ----
    else if (r->doorDown && p->y > (MAP_HEIGHT * TILE_SIZE) - (TILE_SIZE/2)) {
        int idx = FindRoom(d, r->gridX, r->gridY + 1);
        if (idx != -1) {
            d->currentRoom = idx;
            LoadRoomToMap(map);

            // Joga para dentro da nova sala (CIMA)
            p->x = midX;
            p->y = TILE_SIZE * 2.5f;
        }
    }

    // ---- CIMA ----
    else if (r->doorUp && p->y < TILE_SIZE/2) {
        int idx = FindRoom(d, r->gridX, r->gridY - 1);
        if (idx != -1) {
            d->currentRoom = idx;
            LoadRoomToMap(map);

            // Joga para dentro da nova sala (BAIXO)
            p->x = midX;
            p->y = (MAP_HEIGHT - 3.5f) * TILE_SIZE;
        }
    }
}