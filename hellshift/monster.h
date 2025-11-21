#ifndef MONSTER_H
#define MONSTER_H

#include "raylib.h"

#include <stdbool.h>
#include "map.h"

typedef struct Player Player;
typedef struct Projectile Projectile;
typedef struct Map Map;
typedef struct SaveData;

// retorna verdadeiro se algum monstro atingiu o jogador

typedef enum MonsterType{ // tipos de monstro, se for adicionar um novo monstro tem de por um desses aqui
    MONSTER_SKELETON,
    MONSTER_SHADOW_MELEE, // Sombra de adaga
    MONSTER_SHADOW_SPELL // sombra spell
}MonsterType;

typedef struct Monster{ // aqui são as caracteristicas base do tipo "Monstro", são o que vai ser editado par adiferenciar cada um, dps agnt muda isso para adicionar sprites e animações
    Vector2 position;
    MonsterType type;
    int life;
    Color color;
    float speed;
    float activeRange;
}Monster;

typedef struct MonsterNode {
    Monster data;               // dados do monstro
    struct MonsterNode *next;   // ponteiro para o próximo nó da lista
} MonsterNode;

MonsterNode* GetClosestMonsterNode(Vector2 pos, float range);


void SpawnMonster(Vector2 position, MonsterType type);
void UpdateMonsters(Player *p1, Player *p2, int numPlayers, Map *map);
void DrawMonsters(void);
void UnloadMonsters(void);


int CheckMonsterCollision(Rectangle rect);
bool CheckPlayerHit(Vector2 playerPosition, float playerRadius);
int CheckMeleeAttack(Vector2 playerPos, float range, int damage);
int GetMonsterCount(void);
void ExportMonsters(struct SaveData *data);
void ImportMonsters(struct SaveData data);



#endif