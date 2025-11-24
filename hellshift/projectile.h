#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "raylib.h"
#include "player.h" 
#include "map.h"


typedef struct Projectile{
    Vector2 position;
    Vector2 direction;
    float speed;
    float radius;
    Color color;
    int currentFrame;
    float frameTime;

} Projectile;

typedef struct ProjectileNode {
    Projectile data;
    struct ProjectileNode *next;
} ProjectileNode;




// criar um novo projétil no mundo
void SpawnProjectile(Vector2 position, Vector2 direction);

// atualizar a posição de todos os projéteis
void UpdateProjectiles(Map *map, Player *p);

// desenhar todos os projéteis na tela
void DrawProjectiles(void);

// limpar a memória de todos os projéteis no fim do jogo
void UnloadProjectiles(void);

#endif // PROJECTILE_H