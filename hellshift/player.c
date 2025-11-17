#include "player.h"
#include "projectile.h"
#include "map.h"

extern Map mapa;

void InitPlayer(Player *p) {
    p->position = (Vector2){400, 300};
    p->speed = 3.0f;
    p->life = 5;
    p->color = BLUE;
}

//update
void UpdatePlayer(Player *p) { //aqui são funções do jogador, por agora to pondo só movimento basico
    Vector2 oldPos = p->position;
    if (IsKeyDown(KEY_W)) p->position.y -= p->speed;
    if (IsKeyDown(KEY_S)) p->position.y += p->speed;
    if (IsKeyDown(KEY_A)) p->position.x -= p->speed;
    if (IsKeyDown(KEY_D)) p->position.x += p->speed;

    if (CheckMapCollision(mapa, p->position)) {
        p->position = oldPos;
    }    

    if (p->playerclass == CLASS_MAGO && IsKeyPressed(KEY_SPACE)) {
        
        Vector2 direcaoDoTiro = (Vector2){0.0f, -1.0f}; // Atira para cima
        SpawnProjectile(p->position, direcaoDoTiro);
    }

    if (p->Cooldown > 0) {
        p->Cooldown -= GetFrameTime();

        if (((int)(p->damageCooldown * 10)) % 2 == 0) {
            p->color = BLUE; // Cor normal
        } else {
            p->color = RED;  // Cor de dano
        }
    } else {
        p->color = BLUE;
    }    

}

    // Adicionar lógica de colisão com o mapa aqui



void DrawPlayer(Player p) { // faz aparecer na tela, no momento ele vai ser essa bola msm
    DrawCircleV(p.position, 12, p.color);
}