#include "map.h"
#include "monster.h"
#include "projectile.h"
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

//Deixa mais hardcore tlgd (aumenta o numero de salas e inimigos)
static void GenerateDungeonWithDifficulty(Dungeon *d) {

    d->roomCount = 0;

    int maxRooms = 7 + d->floorLevel * 2; 
    if (maxRooms > MAX_ROOMS) maxRooms = MAX_ROOMS;

    Room *start = &d->rooms[0];
    memset(start, 0, sizeof(Room));
    start->gridX = 0;
    start->gridY = 0;
    start->type = ROOM_START;
    start->cleared = true;

    d->roomCount = 1;

    int attempts = 0;

    while (d->roomCount < maxRooms && attempts < 200) {
        attempts++;

        int baseIndex = GetRandomValue(0, d->roomCount - 1);
        Room *cur = &d->rooms[baseIndex];

        int cx = cur->gridX;
        int cy = cur->gridY;

        int dir = GetRandomValue(0,3);
        int nx = cx, ny = cy;

        if (dir == 0) ny--;
        if (dir == 1) ny++;
        if (dir == 2) nx--;
        if (dir == 3) nx++;

        if (FindRoom(d, nx, ny) != -1)
            continue;

        Room *newRoom = &d->rooms[d->roomCount];
        memset(newRoom, 0, sizeof(Room));

        newRoom->gridX = nx;
        newRoom->gridY = ny;
        newRoom->cleared = false;

        if (d->roomCount == maxRooms - 1)
            newRoom->type = ROOM_BOSS;
        else if (GetRandomValue(0, 10) > 8)
            newRoom->type = ROOM_TREASURE;
        else
            newRoom->type = ROOM_NORMAL;

        if (nx > cx) { cur->doorRight = true; newRoom->doorLeft = true; }
        if (nx < cx) { cur->doorLeft  = true; newRoom->doorRight = true; }
        if (ny > cy) { cur->doorDown  = true; newRoom->doorUp = true; }
        if (ny < cy) { cur->doorUp    = true; newRoom->doorDown = true; }

        d->roomCount++;
    }

    for(int i = 0; i < d->roomCount; i++)
        BuildRoom(&d->rooms[i]);

    d->currentRoom = 0;
}

// Spawna inimigos na sala atual, escalando com o andar
static void SpawnRoomEnemies(Map *map) {

    Dungeon *d = &map->dungeon;
    Room *r = &d->rooms[d->currentRoom];

    // START e TREASURE não tem inimigos
    if (r->type == ROOM_START || r->type == ROOM_TREASURE) {
        r->cleared = true;
        return;
    }

    // Se já foi "limpa"/spawnda antes, não faz nada
    if (r->cleared) return;

    // Quantidade de inimigos cresce com o andar
    int enemyCount = 2 + GetRandomValue(0, d->floorLevel);

    for (int i = 0; i < enemyCount; i++) {
        float px = GetRandomValue(80, 700);
        float py = GetRandomValue(80, 400);

        MonsterType tipo = MONSTER_SKELETON;

        // Em andares mais altos, chance de sombras
        if (d->floorLevel >= 3 && GetRandomValue(0, 10) > 7) {
            tipo = MONSTER_SHADOW_MELEE;
        }
        if (d->floorLevel >= 5 && GetRandomValue(0, 10) > 8) {
            tipo = MONSTER_SHADOW_SPELL;
        }

        // Boss room: garantimos um boss mais forte
        if (r->type == ROOM_BOSS) {
            tipo = (i == 0) ? MONSTER_SHADOW_MELEE : MONSTER_SHADOW_SPELL;
        }

        SpawnMonster((Vector2){px, py}, tipo);
    }

    r->cleared = true; // marca que essa sala já foi spawnda
}


/* =========================================
   Funções PÚBLICAS
========================================= */

void LoadMap(Map *map, const char *fileName) {
    (void)fileName;

    map->dungeon.floorLevel = 1;       // começa no andar 1
    GenerateDungeonWithDifficulty(&map->dungeon);

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

    // Agora spawna de acordo com o andar
    SpawnRoomEnemies(map);
    
}

// ================================
// AVANÇAR PARA O PRÓXIMO ANDAR
// ================================
void GoToNextFloor(Map *map, Vector2 *p1Pos, Vector2 *p2Pos, int numPlayers) {
    
    map->dungeon.floorLevel++;

    if (map->dungeon.floorLevel > 7)
        return;

    UnloadMonsters();
    UnloadProjectiles();

    GenerateDungeonWithDifficulty(&map->dungeon);
    LoadRoomToMap(map);

    // Reseta jogadores no centro
    *p1Pos = (Vector2){350, 225};

    if (numPlayers == 2 && p2Pos != NULL) {
        *p2Pos = (Vector2){450, 225};
    }
}