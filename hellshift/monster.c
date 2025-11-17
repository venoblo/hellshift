#include "monster.h"
#include <stdlib.h>
#include "raymath.h"
#include "player.h"
#include "map.h"

extern Map mapa;

static MonsterNode *listaDeMonstros = NULL;

// desenhar um monstro
static void DrawOneMonster(Monster m) {
    DrawCubeV((Vector3){m.position.x, m.position.y, 0.0f}, (Vector3){15.0f, 15.0f, 15.0f}, m.color);
}

// da update em um monstro
static void UpdateOneMonster(Monster *m, Vector2 playerPos) {
    
    Vector2 oldPos = m->position;
    // calcula o vetor de direção 
    Vector2 direction = Vector2Subtract(playerPos, m->position);
    
    // normaliza o vetor
    direction = Vector2Normalize(direction);
    
    // move o monstro em direção ao jogador, multiplicado pela velocidade
    m->position.x += direction.x * m->speed;
    m->position.y += direction.y * m->speed;

    if (CheckMapCollision(mapa, m->position)) {
        m->position = oldPos;
    }
}

void SpawnMonster(Vector2 position, MonsterType type) {
    // 1. Aloca memória para um novo NÓ
    MonsterNode *novoMonstro = (MonsterNode *)malloc(sizeof(MonsterNode));
    if (novoMonstro == NULL) {
        // Falha ao alocar memória!
        return;
    }

    // 2. Preenche os dados (data) do monstro
    novoMonstro->data.position = position;
    novoMonstro->data.type = type;
    novoMonstro->data.life = 3;
    novoMonstro->data.speed = 1.0f; // Dando uma velocidade padrão
    novoMonstro->data.color = RED;

    // 3. Insere o novo nó no INÍCIO da lista
    novoMonstro->next = listaDeMonstros;
    listaDeMonstros = novoMonstro;
}

void UpdateMonsters(Player *player) {

    MonsterNode *current = listaDeMonstros; // Começa no primeiro nó
    
    // "Enquanto não chegamos ao fim da lista..."
    while (current != NULL) {
        // Atualiza o monstro atual
        UpdateOneMonster(&(current->data), player->position);
        
        // Avança para o próximo nó
        current = current->next;
    }
}

void DrawMonsters(void) {

    MonsterNode *current = listaDeMonstros;
    // DrawCubeV(m.position, 15, m.color);
    while (current != NULL) {
        // Desenha o monstro atual
        DrawOneMonster(current->data);
        
        // Avança para o próximo nó
        current = current->next;
    }
}

void UnloadMonsters(void) {
    MonsterNode *current = listaDeMonstros;
    MonsterNode *next;

    while (current != NULL) {
        next = current->next; 
        free(current);        
        current = next;       
    }
    
    listaDeMonstros = NULL;
}

#include "monster.h"
// ... outros includes

// ... (todas as suas funções existentes: DrawOne, UpdateOne, Spawn, Update, Draw, Unload) ...

// NOVA FUNÇÃO PÚBLICA (Implementação da deleção de nó do monstro)
int CheckMonsterCollision(Rectangle rect) {
    
    MonsterNode *prev = NULL;
    MonsterNode *current = listaDeMonstros;

    while (current != NULL) {
        MonsterNode *nextNode = current->next;
        
        // Cria o "Hitbox" do monstro
        Rectangle monsterRect = {
            .x = current->data.position.x - 7.5f, // (Metade de 15)
            .y = current->data.position.y - 7.5f, // (Metade de 15)
            .width = 15,
            .height = 15
        };

        // CHECA A COLISÃO
        if (CheckCollisionRecs(rect, monsterRect)) {
            // ACERTOU!
            
            // (Futuramente: current->data.life-- e só remove se a vida chegar a 0)
            
            // Remove o monstro da lista
            if (prev == NULL) {
                // É o primeiro da lista
                listaDeMonstros = nextNode;
            } else {
                // Está no meio ou fim
                prev->next = nextNode;
            }
            
            free(current); // Libera a memória do monstro
            return 100; // Retorna 'true' (um monstro foi atingido)
        }
        
        // Avança na lista
        prev = current;
        current = nextNode;
    }
    
    return 0; // Nenhum monstro foi atingido
}

bool CheckPlayerHit(Vector2 playerPosition, float playerRadius) {
    MonsterNode *current = listaDeMonstros;
    
    while (current != NULL) {
        // Hitbox do Monstro
        Rectangle monsterRect = {
            .x = current->data.position.x - 7.5f,
            .y = current->data.position.y - 7.5f,
            .width = 15,
            .height = 15
        };
        
        // Raylib tem função para checar Círculo com Retângulo!
        if (CheckCollisionCircleRec(playerPosition, playerRadius, monsterRect)) {
            return true; // Tocou!
        }
        
        current = current->next;
    }
    return false;
}