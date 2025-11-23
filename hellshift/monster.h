#ifndef MONSTER_H
#define MONSTER_H

#include "raylib.h"
#include "map.h"
#include <stdbool.h>
#include "map.h"

struct Player;
struct Projectile;
struct SaveData;


typedef enum MonsterAnimState {
    MONSTER_IDLE,
    MONSTER_WALK,
    MONSTER_ATTACK,
    MONSTER_HURT,
    MONSTER_DEATH
} MonsterAnimState;

typedef enum SkeletonVariant {
    SKEL_NORMAL = 0,
    SKEL_ARMORED,
    SKEL_GREATSWORD,
    SKEL_VARIANT_COUNT
} SkeletonVariant;

// speed do attack por variante
float GetSkeletonAttackAnimSpeed(SkeletonVariant v);
int GetSkeletonMaxFrames(SkeletonVariant v, MonsterAnimState st);


typedef enum MonsterType{ // tipos de monstro, se for adicionar um novo monstro tem de por um desses aqui
    MONSTER_SKELETON,
    MONSTER_SHADOW_MELEE, // Sombra de adaga
    MONSTER_SHADOW_SPELL // sombra spell
}MonsterType;

typedef struct Monster {
    Vector2 position;
    SkeletonVariant skelVariant; // só usado se type == MONSTER_SKELETON
    MonsterType type;
    int life;
    Color color;
    float speed;
    float activeRange;
    bool isPossessed; 

    // --- sprite ---
    MonsterAnimState state;
    int currentFrame;
    float frameTime;
    int facingDirection; // 1 direita, -1 esquerda

    Texture2D texIdle;
    Texture2D texWalk;
    Texture2D texAttack;
    Texture2D texHurt;
    Texture2D texDeath;
} Monster;


typedef struct MonsterNode {
    Monster data;               // dados do monstro
    struct MonsterNode *next;   // ponteiro para o próximo nó da lista
} MonsterNode;

MonsterNode* GetClosestMonsterNode(Vector2 pos, float range);


void SpawnMonster(Vector2 position, MonsterType type);
void UpdateMonsters(struct Player *p1, struct Player *p2, int numPlayers, Map *map);
void DrawMonsters(void);
void UnloadMonsters(void);


int CheckMonsterCollision(Rectangle rect);
bool CheckPlayerHit(Vector2 playerPosition, float playerRadius);
int CheckMeleeAttack(Vector2 playerPos, float range, int damage);
int GetMonsterCount(void);
void ExportMonsters(struct SaveData *data);
void ImportMonsters(struct SaveData data);



#endif