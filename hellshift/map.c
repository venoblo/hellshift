#include "map.h"
#include "monster.h"
#include "projectile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================
   Funções INTERNAS
========================================= */

static MonsterType RollBaseEnemyByFloor(int floor) {
    if (floor <= 1) return MONSTER_SKELETON;

    if (floor == 2) {
        int r = GetRandomValue(0, 99);
        return (r < 70) ? MONSTER_SKELETON : MONSTER_SLIME; // 70/30
    }

    // floor >= 3
    int r = GetRandomValue(0, 99);
    if (r < 50) return MONSTER_SKELETON; // 50%
    if (r < 75) return MONSTER_SLIME;    // 25%
    return MONSTER_ORC;                  // 25%
}

// Copia a sala atual para map->tiles
static void LoadRoomToMap(Map *map) {
    Room *r = &map->dungeon.rooms[map->dungeon.currentRoom];

    r->visited = true;

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

        // Cantos
        if (x == 0 && y == 0)
            r->tiles[y][x] = TILE_WALL_TL;
        else if (x == MAP_WIDTH - 1 && y == 0)
            r->tiles[y][x] = TILE_WALL_TR;
        else if (x == 0 && y == MAP_HEIGHT - 1)
            r->tiles[y][x] = TILE_WALL_BL;
        else if (x == MAP_WIDTH - 1 && y == MAP_HEIGHT - 1)
            r->tiles[y][x] = TILE_WALL_BR;

        // Bordas
        else if (y == 0)
            r->tiles[y][x] = TILE_WALL_TOP;
        else if (y == MAP_HEIGHT - 1)
            r->tiles[y][x] = TILE_WALL_BOTTOM;
        else if (x == 0)
            r->tiles[y][x] = TILE_WALL_LEFT;
        else if (x == MAP_WIDTH - 1)
            r->tiles[y][x] = TILE_WALL_RIGHT;

        // Chão
        else
            r->tiles[y][x] = TILE_EMPTY;
        }
    }

    int midX = MAP_WIDTH / 2;
    int midY = MAP_HEIGHT / 2;

    if (r->doorUp)    r->tiles[0][midX] = TILE_DOOR_CLOSED;
    if (r->doorDown)  r->tiles[MAP_HEIGHT - 1][midX] = TILE_DOOR_CLOSED;
    if (r->doorLeft)  r->tiles[midY][0] = TILE_DOOR_CLOSED;
    if (r->doorRight) r->tiles[midY][MAP_WIDTH - 1] = TILE_DOOR_CLOSED;

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
    int bossIndex = 0;
    int maxDist = 0;

    for (int i = 0; i < d->roomCount; i++) {
        int dist = abs(d->rooms[i].gridX) + abs(d->rooms[i].gridY);

        if (dist > maxDist) {
            maxDist = dist;
            bossIndex = i;
        }

        d->rooms[i].type = ROOM_NORMAL;
    }

    d->rooms[bossIndex].type = ROOM_BOSS;
    d->rooms[d->roomCount - 1].cleared = false;

    for (int i = 0; i < d->roomCount; i++)
        BuildRoom(&d->rooms[i]);

    d->currentRoom = 0;
}

//Deixa mais hardcore tlgd (aumenta o numero de salas e inimigos conforme passa de andar)
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

        MonsterType tipo;

        // floor 1: só skeleton
        if (d->floorLevel == 1) {
            tipo = MONSTER_SKELETON;
        }
        // floor 2: 55/45 skeleton/slime
        else if (d->floorLevel == 2) {
            int r = GetRandomValue(0, 99);
            tipo = (r < 55) ? MONSTER_SKELETON : MONSTER_SLIME;
        }
        // floor 3+: 60% orc, 20% skeleton, 20% slime
        else {
            int r = GetRandomValue(0, 99);
            if (r < 60)      tipo = MONSTER_ORC;
            else if (r < 80) tipo = MONSTER_SKELETON;
            else             tipo = MONSTER_SLIME;
        }

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

static MonsterType RollEnemyTypeByFloor(int floor) {
    int r = GetRandomValue(0, 99);

    if (floor <= 1) {
        return MONSTER_SKELETON; // só skeleton
    }

    if (floor == 2) {
        // skeleton + slime (slime começa aparecer)
        return (r < 55) ? MONSTER_SKELETON : MONSTER_SLIME;  // 70/30
    }

    // floor >= 3: orcs maioria
    if (r < 60) return MONSTER_ORC;        // 70% orc
    if (r < 80) return MONSTER_SKELETON;   // 15% esqueleto
    return MONSTER_SLIME;                 // 15% slime
}


/* =========================================
   Funções PÚBLICAS
========================================= */

Texture2D wallTex;
Texture2D floorTex;
Texture2D doorTex;

void LoadMapTextures(void)
{
    wallTex = LoadTexture("resources/tiles/decorative_cracks_walls.png");
    floorTex = LoadTexture("resources/tiles/decorative_cracks_floor.png");
    doorTex = LoadTexture("resources/tiles/doors_lever_chest_animation.png");

    printf("Wall ID: %d\n", wallTex.id);
    printf("Floor ID: %d\n", floorTex.id);
    printf("Door ID: %d\n", doorTex.id);
}

void UnloadMapTextures(void)
{
    UnloadTexture(wallTex);
    UnloadTexture(floorTex);
    UnloadTexture(doorTex);
}

void LoadMap(Map *map, const char *fileName) {
    (void)fileName;

    SetTextureFilter(doorTex, TEXTURE_FILTER_POINT);
    SetTextureFilter(wallTex, TEXTURE_FILTER_POINT);
    SetTextureFilter(floorTex, TEXTURE_FILTER_POINT);

    map->dungeon.floorLevel = 1;       // começa no andar 1
    GenerateDungeonWithDifficulty(&map->dungeon);

    LoadRoomToMap(map);

    map->dungeon.rooms[0].visited = true; // sala inicial mascada como visitada
}

static Rectangle GetTile(int col, int row)
{
    return (Rectangle){
        col * SPRITE_SIZE,
        row * SPRITE_SIZE,
        SPRITE_SIZE,
        SPRITE_SIZE
    };
}

static Rectangle GetDoorTile(int col, int row)
{
    return (Rectangle){
        col * 32,   // porta é 32x32 nesse sheet
        row * 24,
        32,
        24
    };
}

void DrawMap(Map map)
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            Rectangle tileRec = {
                .x = x * TILE_SIZE + MAP_OFFSET_X,
                .y = y * TILE_SIZE + MAP_OFFSET_Y,
                .width = TILE_SIZE,
                .height = TILE_SIZE
            };

            Rectangle dest = { tileRec.x, tileRec.y, TILE_SIZE, TILE_SIZE };

            int t = map.tiles[y][x];
            Rectangle src;

            // 1) PAREDES (usam wallTex)
            if (t >= TILE_WALL_TOP && t <= TILE_WALL_BR)
            {
                switch (t)
                {
                    case TILE_WALL_TOP:    src = GetTile(0, 0); break;
                    case TILE_WALL_BOTTOM: src = GetTile(1, 0); break;
                    case TILE_WALL_LEFT:   src = GetTile(2, 0); break;
                    case TILE_WALL_RIGHT:  src = GetTile(3, 0); break;
                    case TILE_WALL_TL:     src = GetTile(4, 0); break;
                    case TILE_WALL_TR:     src = GetTile(5, 0); break;
                    case TILE_WALL_BL:     src = GetTile(6, 0); break;
                    case TILE_WALL_BR:     src = GetTile(7, 0); break;
                    default:               src = GetTile(0, 0); break;
                }

                DrawTexturePro(
                    wallTex,
                    src,
                    dest,
                    (Vector2){0,0},
                    0,
                    WHITE
                );
            }

            // PORTA ABERTA
            else if (t == TILE_DOOR)
            {
                // ajusta (col,row) conforme o tile de porta da sua spritesheet
                src = GetDoorTile(4, 0);
                DrawTexturePro(
                    doorTex,
                    src,
                    dest,
                    (Vector2){0,0},
                    0,
                    WHITE
                );
            }

            // PORTA FECHADA
            else if (t == TILE_DOOR_CLOSED)
            {
                src = GetDoorTile(0, 0);
                DrawTexturePro(
                    doorTex,
                    src,
                    dest,
                    (Vector2){0,0},
                    0,
                    WHITE
                );
            }

            // 3) ARMADILHA (só cor por enquanto)
            else if (t == TILE_TRAP)
            {
                DrawRectangleRec(tileRec, MAROON);
            }

            // 4) CHÃO (usa floorTex)
            else
            {
                // ajusta (col,row) pro tile de piso que você quiser
                src = GetTile(0, 0);
                DrawTexturePro(
                    floorTex,
                    src,
                    dest,
                    (Vector2){0,0},
                    0,
                    WHITE
                );
            }
        }
    }
}

bool CheckMapCollision(Map map, Vector2 worldPos) {

    float localX = worldPos.x - MAP_OFFSET_X;
    float localY = worldPos.y - MAP_OFFSET_Y;

    int mapX = (int)(localX / TILE_SIZE);
    int mapY = (int)(localY / TILE_SIZE);

    if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT){
        return true;
    }
    int tile = map.tiles[mapY][mapX];

    return (tile >= TILE_WALL_TOP && tile <= TILE_WALL_BR);

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
void CheckRoomTransition(Map *map, Player *p1, Player *p2, int numPlayers) {

    char from = ' ';

    Dungeon *d = &map->dungeon;
    Room *r = &d->rooms[d->currentRoom];

    // abre as portas dps que matar tudo
    if (GetMonsterCount() == 0) {
        int midX = MAP_WIDTH / 2;
        int midY = MAP_HEIGHT / 2;

        if (r->doorUp)    map->tiles[0][midX] = TILE_DOOR;
        if (r->doorDown)  map->tiles[MAP_HEIGHT - 1][midX] = TILE_DOOR;
        if (r->doorLeft)  map->tiles[midY][0] = TILE_DOOR;
        if (r->doorRight) map->tiles[midY][MAP_WIDTH - 1] = TILE_DOOR;
    }

    // Não pode sair se ainda tem inimigos
    if (GetMonsterCount() > 0) return;

    int rightLimit = MAP_WIDTH * TILE_SIZE - 10;
    int leftLimit  = 10;
    int downLimit  = MAP_HEIGHT * TILE_SIZE - 10;
    int upLimit    = 10;

    int nextIndex = -1;

    // Só jogador VIVO pode ativar porta
    if (!p1->ghost) {
        if (r->doorRight && p1->position.x > rightLimit) {
            nextIndex = FindRoom(d, r->gridX + 1, r->gridY);
            from = 'R';
        }
        else if (r->doorLeft && p1->position.x < leftLimit) {
            nextIndex = FindRoom(d, r->gridX - 1, r->gridY);
            from = 'L';
        }
        else if (r->doorDown && p1->position.y > downLimit) {
            nextIndex = FindRoom(d, r->gridX, r->gridY + 1);
            from = 'D';
        }
        else if (r->doorUp && p1->position.y < upLimit) {
            nextIndex = FindRoom(d, r->gridX, r->gridY - 1);
            from = 'U';
        }
    }

    // Em 2 jogadores, verifica também o P2
    if (numPlayers == 2 && nextIndex == -1 && !p2->ghost) {
        if (r->doorRight && p2->position.x > rightLimit) {
            nextIndex = FindRoom(d, r->gridX + 1, r->gridY);
            from = 'R';
        }
        else if (r->doorLeft && p2->position.x < leftLimit) {
            nextIndex = FindRoom(d, r->gridX - 1, r->gridY);
            from = 'L';
        }
        else if (r->doorDown && p2->position.y > downLimit) {
            nextIndex = FindRoom(d, r->gridX, r->gridY + 1);
            from = 'D';
        }
        else if (r->doorUp && p2->position.y < upLimit) {
            nextIndex = FindRoom(d, r->gridX, r->gridY - 1);
            from = 'U';
        }
    }

    if (nextIndex == -1) return;

    // LIMPA MONSTROS DA SALA ANTERIOR
    UnloadMonsters();

    // TROCA DE SALA
    d->currentRoom = nextIndex;
    LoadRoomToMap(map);

    //VERIFICA SE JÁ VISITOU A SALA
    d->rooms[d->currentRoom].visited = true;

    // SPAWN DA NOVA SALA
    Room *newRoom = &d->rooms[d->currentRoom];

    newRoom->visited = true;
    newRoom->discovered = true;

    // Descobre salas adjacentes
    for (int i = 0; i < d->roomCount; i++) {
        Room *r = &d->rooms[i];

        if (abs(r->gridX - newRoom->gridX) + abs(r->gridY - newRoom->gridY) == 1) {
            r->discovered = true;
        }
    }

    int midX = (MAP_WIDTH  * TILE_SIZE) / 2;
    int midY = (MAP_HEIGHT * TILE_SIZE) / 2;

    // TELEPORTA OS JOGADORES JUNTOS
    if (from == 'R') {
    p1->position = (Vector2){TILE_SIZE * 2, midY};
    if (numPlayers == 2) 
        p2->position = (Vector2){TILE_SIZE * 3, midY};
    }
    else if (from == 'L') {
        p1->position = (Vector2){(MAP_WIDTH - 3) * TILE_SIZE, midY};
        if (numPlayers == 2) 
            p2->position = (Vector2){(MAP_WIDTH - 4) * TILE_SIZE, midY};
    }
    else if (from == 'D') {
        p1->position = (Vector2){midX, TILE_SIZE * 2};
        if (numPlayers == 2) 
            p2->position = (Vector2){midX + 40, TILE_SIZE * 2};
    }
    else if (from == 'U') {
        p1->position = (Vector2){midX, (MAP_HEIGHT - 3) * TILE_SIZE};
        if (numPlayers == 2) 
            p2->position = (Vector2){midX + 40, (MAP_HEIGHT - 3) * TILE_SIZE};
    }

    if (!newRoom->cleared) {
        SpawnRoomEnemies(map);  
    }

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

// ===========
// Minimapa
// ===========
Color GetRoomColor(Room *r, bool isCurrent)
{
    if (isCurrent) 
        return BLUE;

    if (!r->visited) 
        return (Color){20, 20, 20, 255};

    switch (r->type)
    {
        case ROOM_START:    return GREEN;
        case ROOM_NORMAL:   return GRAY;
        case ROOM_TREASURE: return GOLD;
        case ROOM_BOSS:      return RED;
        default:             return DARKGRAY;
    }
}

static Vector2 GetMiniMapPosition(Room *r, Room *current, int centerX, int centerY, int tileSize)
{
    int dx = r->gridX - current->gridX;
    int dy = r->gridY - current->gridY;

    float px = centerX + dx * (tileSize + 6);
    float py = centerY + dy * (tileSize + 6);

    return (Vector2){ px, py };
}

void DrawMiniMap(Map *map, bool expanded)
{
    Dungeon *d = &map->dungeon;

    int tileSize = expanded ? 16 : 6;
    int margin = 15;

    int bgWidth  = expanded ? 520 : 140;
    int bgHeight = expanded ? 300 : 100;

    int originX = expanded ? (GetScreenWidth()/2 - bgWidth/2) 
                           : (GetScreenWidth() - bgWidth - margin);

    int originY = expanded ? (GetScreenHeight()/2 - bgHeight/2) 
                           : 60;

    int centerX = originX + bgWidth/2;
    int centerY = originY + bgHeight/2;

    int viewDistance = expanded ? 999 : 1;

    DrawRectangle(originX, originY, bgWidth, bgHeight, (Color){10,10,10,220});
    DrawRectangleLines(originX, originY, bgWidth, bgHeight, RED);

    Room *current = &d->rooms[d->currentRoom];

    for (int i = 0; i < d->roomCount; i++)
    {
        Room *r = &d->rooms[i];

        if (!r->discovered){
            continue;
        }

        int dx = r->gridX - current->gridX;
        int dy = r->gridY - current->gridY;

        if (!expanded && (abs(dx) > viewDistance || abs(dy) > viewDistance)){
            continue;
        }

        int px = centerX + dx * (tileSize + 6) - tileSize/2;
        int py = centerY + dy * (tileSize + 6) - tileSize/2;

        if (px < originX || py < originY ||
            px + tileSize > originX + bgWidth ||
            py + tileSize > originY + bgHeight){
            continue;
        }

        Color color = GetRoomColor(r, i == d->currentRoom);
        DrawRectangle(px, py, tileSize, tileSize, color);

        if (expanded){
            DrawRectangleLines(px, py, tileSize, tileSize, BLACK);
        }
    }
}