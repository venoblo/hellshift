#include "player.h"
#include "projectile.h"
#include "map.h"
#include "monster.h"
#include <stdlib.h>

void InitPlayerClassStats(Player *p) {
    p->damageCooldown = 0.0f;
    p->ghost = false;
    p->attackTimer = 0.0f; 

    p->originalColor = p->color;

    // Lógica SWITCH/CASE para aplicar as estatísticas
    if (p->playerclass == CLASS_GUERREIRO) {
        // Stats do Guerreiro (Tanque)
        p->maxLife = 100;
        p->life = 100;
        p->speed = 2.0f; // Mais lento
    } 
    else if (p->playerclass == CLASS_MAGO) {
        // Stats do Mago (Vidro/Dano)
        p->maxLife = 80;
        p->life = 80;
        p->speed = 2.5f; // Mais rápido
    }
    p->trapActive = false;
}

//update
void UpdatePlayer(Player *p, Map *map) { //aqui são funções do jogador, por agora to pondo só movimento basico
    
    p->trapActive = false;
    if (p->isPossessing && p->possessedMonster != NULL) {
        MonsterNode *monstro = (MonsterNode*)p->possessedMonster;
        
        // Controla o monstro!
        if (IsKeyDown(p->keyUp))    monstro->data.position.y -= monstro->data.speed;
        if (IsKeyDown(p->keyDown))  monstro->data.position.y += monstro->data.speed;
        if (IsKeyDown(p->keyLeft))  monstro->data.position.x -= monstro->data.speed;
        if (IsKeyDown(p->keyRight)) monstro->data.position.x += monstro->data.speed;
        
        // A câmera/jogador segue o monstro
        p->position = monstro->data.position; 
        p->ghost = false; // Tecnicamente não é fantasma visualmente
        
        // Se o monstro morrer (vida < 0), o jogador é expulso
        if (monstro->data.life <= 0) {
            p->isPossessing = false;
            p->possessedMonster = NULL;
            p->ghost = true; // Volta a ser fantasma
            // A morte real do monstro é tratada no monster.c ou main loop
        }
        return;
    }

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

        if (IsKeyPressed(p->keyAction)) {
            // 1. Tenta Possuir
            MonsterNode *alvo = GetClosestMonsterNode(p->position, 50.0f);
            if (alvo != NULL) {
                p->isPossessing = true;
                p->possessedMonster = alvo;
                // Opcional: Mudar cor do monstro para indicar possessão
                alvo->data.color = p->originalColor; 
                return;
            }

            // 2. Tenta Ativar Armadilha
            if (CheckTrapInteraction(map, p->position)) {
                p->trapActive = true;
                // Explode! Causa dano em área aos monstros ou ao outro jogador?
                // "causando dano no jogador que está vivo" -> Ok!
            }
        }    

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

    if (IsKeyPressed(p->keyAction)) {
        
        // 1. Ataque Mago (Projétil)
        if (p->playerclass == CLASS_MAGO) {
            Vector2 direcaoDoTiro = (Vector2){0.0f, -1.0f}; 
            SpawnProjectile(p->position, direcaoDoTiro);
        }
        // 2. Ataque Guerreiro (Melee)
        else if (p->playerclass == CLASS_GUERREIRO) {
            float alcanceEspada = 20.0f;
            int dano = 15; 
            
            // Chama a função que criamos no monster.c
            int pontos = CheckMeleeAttack(p->position, alcanceEspada, dano);
            
            p->score += pontos; // Soma a pontuação
            p->attackTimer = 0.2f; // Ativa o efeito visual
        }
    }
    if (p->attackTimer > 0){
        p->attackTimer -= GetFrameTime();
    }

    if (p->playerclass == CLASS_MAGO && IsKeyPressed(p->keyAction)) {
        
        Vector2 direcaoDoTiro = (Vector2){0.0f, -1.0f}; 
        SpawnProjectile(p->position, direcaoDoTiro);
    }

    else if (p->playerclass == CLASS_GUERREIRO && IsKeyPressed(p->keyAction)) {
        // Define o alcance da espada (ex: 60 pixels ao redor dele)
        float alcanceEspada = 60.0f;
        int dano = 3; // Mata hit-kill esqueletos (que tem 3 de vida)
        
        // Chama a função que criamos no monster.c
        int pontos = CheckMeleeAttack(p->position, alcanceEspada, dano);
        
        p->score += pontos; // Soma a pontuação
        
        // Define um tempo visual de ataque (para desenhar o efeito depois)
        p->attackTimer = 0.2f; // Dura 0.2 segundos
    }

    if (p->damageCooldown > 0) {
        p->damageCooldown -= GetFrameTime();
        if (((int)(p->damageCooldown * 10)) % 2 == 0) {
            p->color = p->originalColor;
        } else {
            p->color = RED;  
        }
    }
    else{
        p->color = p->originalColor;
    }
    

}




void DrawPlayer(Player p) { 
    if (p.isPossessing) return;

    if (p.playerclass == CLASS_GUERREIRO && p.attackTimer > 0) {
        DrawCircleLines(p.position.x, p.position.y, 20, WHITE); // Alcance visual
    }
    if (p.ghost) {
        DrawCircleV(p.position, 12, Fade(p.color, 0.3f));
    } else {
        DrawCircleV(p.position, 12, p.color);
        DrawCircleLines(p.position.x, p.position.y, 12, BLACK); // Borda ajuda a ver
    }
}