#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "raylib.h"

typedef struct Projectil{
    Vector2 position;
    Vector2 direction;
    float speed;
    float radius;
    Color color;
} Projectil;

typedef struct ProjectilNode {
    Projectil data;
    struct ProjectilNode *next;
} ProjectilNode;




// criar um novo projétil no mundo
void SpawnProjectile(Vector2 position, Vector2 direction);

// atualizar a posição de todos os projéteis
void UpdateProjectiles(void);

// desenhar todos os projéteis na tela
void DrawProjectiles(void);

// limpar a memória de todos os projéteis no fim do jogo
void UnloadProjectiles(void);

#endif // PROJECTILE_H