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
        
        // Guarda o próximo nó ANTES de potencialmente deletar o atual
        ProjectilNode *nextNode = current->next; 

        // Checa colisão com o mapa
        bool colidiumapa = CheckMapCollision(*map, current->data.position);

        Rectangle projRect = {
            .x = current->data.position.x - current->data.radius,
            .y = current->data.position.y - current->data.radius,
            .width = current->data.radius * 2,
            .height = current->data.radius * 2
        };
        
        // (Futuramente, adicione: OU colidiu com monstro)
        
        if (colidiu) {
            // Se colidiu, remove o nó da lista

            if (prev == NULL) {
                // Caso 1: O nó a ser removido é o PRIMEIRO da lista
                listaDeProjeteis = nextNode;
            } else {
                // Caso 2: O nó está no meio ou fim da lista
                prev->next = nextNode;
            }
            
            free(current); // Libera a memória!
            
        } else {
            // Se não colidiu, este nó se torna o "anterior" para a próxima iteração
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