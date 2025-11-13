#ifndef MONSTER_H
#define MONSTER_H

#include "raylib.h"
#include "player.h"

typedef enum MonsterType{ // tipos de monstro, se for adicionar um novo monstro tem de por um desses aqui
    MONSTER_SLIME,
    MONSTER_SKELETON,
    MONSTER_ZOMBIE
}MonsterType;

typedef struct Monster{ //qui são as caracteristicas base do tipo "Monstro", são o que vai ser editado par adiferenciar cada um, dps agnt muda isso para adicionar sprites e animações
    Vector2 position;
    MonsterType type;
    int life;
    Color color;
    float speed;
}Monster;

typedef struct MonsterNode {
    Monster data;               // dados do monstro
    struct MonsterNode *next;   // ponteiro para o próximo nó da lista
} MonsterNode;

void SpawnMonster(Vector2 position, MonsterType type);
void UpdateMonsters(Player *player);
void DrawMonsters(void);
void UnloadMonsters(void);

bool CheckMonsterHit(Projectil projectile);
bool CheckMonsterCollision(Rectangle rect);

#endif
