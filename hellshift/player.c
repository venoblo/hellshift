#include "player.h"
#include "projectile.h"
#include "map.h"

void InitPlayerClassStats(Player *p) {
    // Garante que o score/cooldown sejam resetados antes de começar
    p->score = 0;
    p->damageCooldown = 0.0f;
    p->ghost = false;

    Color baseColor = p->color;

    // Lógica SWITCH/CASE para aplicar as estatísticas
    if (p->playerclass == CLASS_GUERREIRO) {
        // Stats do Guerreiro (Tanque)
        p->maxLife = 8;
        p->life = 8;
        p->speed = 2.0f; // Mais lento, mais durável
        p->color = baseColor; // Mudar a cor para indicar a classe
    } 
    else if (p->playerclass == CLASS_MAGO) {
        // Stats do Mago (Vidro/Dano)
        p->maxLife = 4;
        p->life = 4;
        p->speed = 4.0f; // Mais rápido, menos vida
        p->color = baseColor;
    }
}

//update
void UpdatePlayer(Player *p, Map *map) { //aqui são funções do jogador, por agora to pondo só movimento basico
    
    if (p->ghost) {

        float ghostSpeed = p->speed * 1.5f; 

        if (IsKeyDown(p->keyUp))    p->position.y -= ghostSpeed;
        if (IsKeyDown(p->keyDown))  p->position.y += ghostSpeed;
        if (IsKeyDown(p->keyLeft))  p->position.x -= ghostSpeed;
        if (IsKeyDown(p->keyRight)) p->position.x += ghostSpeed;
        
        // Mantém o fantasma dentro da tela (opcional, mas bom para não perder ele)
        // Assumindo 800x450
        if (p->position.x < 0) p->position.x = 0;
        if (p->position.y < 0) p->position.y = 0;
        if (p->position.x > 800) p->position.x = 800;
        if (p->position.y > 450) p->position.y = 450;

        return; // Sai da função (não faz mais nada)
    }
    
    
    Vector2 oldPos = p->position;
    if (IsKeyDown(p->keyUp)) p->position.y -= p->speed;
    if (IsKeyDown(p->keyDown)) p->position.y += p->speed;
    if (IsKeyDown(p->keyLeft)) p->position.x -= p->speed;
    if (IsKeyDown(p->keyRight)) p->position.x += p->speed;

    if (CheckMapCollision(*map, p->position)) {
        p->position = oldPos;
    }

    if (p->playerclass == CLASS_MAGO && IsKeyPressed(p->keyAction)) {
        
        Vector2 direcaoDoTiro = (Vector2){0.0f, -1.0f}; 
        SpawnProjectile(p->position, direcaoDoTiro);
    }

    if (p->damageCooldown > 0) {
        p->damageCooldown -= GetFrameTime();
        if (((int)(p->damageCooldown * 10)) % 2 == 0) {
            p->color = BLUE; 
        } else {
            p->color = RED;  
        }
    }
    else{
        p->color = BLUE;
    }
    

}




void DrawPlayer(Player p) { // faz aparecer na tela, no momento ele vai ser essa bola msm
    if (p.ghost) {
        // Desenha transparente (Alpha 0.3) se for fantasma
        DrawCircleV(p.position, 12, Fade(p.color, 0.3f));
    } else {
    DrawCircleV(p.position, 12, p.color);
    }
}