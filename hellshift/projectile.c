#include "projectile.h"
#include "map.h"
#include <stdlib.h> // Para malloc/free
#include "monster.h" 
#include "player.h"

// A lista de projéteis
static ProjectilNode *listaDeProjeteis = NULL;

// Função privada para desenhar UM projétil
static void DrawOneProjectile(Projectil p) {
    DrawCircleV(p.position, p.radius, p.color);
}

// Função privada para atualizar um projétil
static void UpdateOneProjectile(Projectil *p) {
    // Move o projétil na sua direção
    p->position.x += p->direction.x * p->speed;
    p->position.y += p->direction.y * p->speed;
    
    // (Aqui você adicionará lógica para "morrer" se sair da tela)
}

// --- Funções Públicas ---

void SpawnProjectile(Vector2 position, Vector2 direction) {
    //  alocar memória
    ProjectilNode *novo = (ProjectilNode *)malloc(sizeof(ProjectilNode));
    if (novo == NULL) return;

    // preencher dados
    novo->data.position = position;
    novo->data.direction = direction;
    novo->data.speed = 5.0f;
    novo->data.radius = 5.0f;
    novo->data.color = YELLOW;
    
    // inserir na lista
    novo->next = listaDeProjeteis;
    listaDeProjeteis = novo;
}

void UpdateProjectiles(Map *map, Player *p) {

    ProjectilNode *prev = NULL;
    ProjectilNode *current = listaDeProjeteis;
    
    while (current != NULL) {
        UpdateOneProjectile(&(current->data));
        
        ProjectilNode *nextNode = current->next; 

        // 1. Checa colisão com o MAPA
        bool colidiuComMapa = CheckMapCollision(*map, current->data.position);

        // 2. Prepara Hitbox do Projétil
        Rectangle projRect = {
            .x = current->data.position.x - current->data.radius,
            .y = current->data.position.y - current->data.radius,
            .width = current->data.radius * 2,
            .height = current->data.radius * 2
        };
        
        
        int pontosGanhos = CheckMonsterCollision(projRect);
        
        // Se matou monstro, soma pontos no Player (Isso resolve o erro "unused p")
        if (pontosGanhos > 0) {
             p->score += pontosGanhos; 
        }
        
        // 4. Verifica se deve destruir o projétil
        // Se bateu no mapa OU ganhou pontos (acertou monstro)
        if (colidiuComMapa || pontosGanhos > 0) {
            
            // Lógica de remoção da lista
            if (prev == NULL) {
                listaDeProjeteis = nextNode;
            } else {
                prev->next = nextNode;
            }
            
            free(current); 
            
        } else {
            // Se não colidiu, avança o prev
            prev = current;
        }

        // Avança para o próximo nó
        current = nextNode;
    }
}

void DrawProjectiles(void) {
    ProjectilNode *current = listaDeProjeteis;
    while (current != NULL) {
        DrawOneProjectile(current->data);
        current = current->next;
    }
}

void UnloadProjectiles(void) {
    ProjectilNode *current = listaDeProjeteis;
    ProjectilNode *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    listaDeProjeteis = NULL;
}