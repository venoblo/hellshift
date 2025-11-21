#include "monster.h"
#include <stdlib.h>
#include "raymath.h" 
#include "player.h"
#include "map.h"

static MonsterNode *listaDeMonstros = NULL;

// Desenho melhorado para identificar quem é quem
static void DrawOneMonster(Monster m) {
    if (m.type == MONSTER_SKELETON) {
        // Esqueleto: Quadrado Bege
        DrawRectangleV(m.position, (Vector2){30, 30}, BEIGE);
        DrawRectangleLines(m.position.x, m.position.y, 30, 30, BLACK);
    } else {
        // Sombras: Quadrado Roxo Escuro
        DrawRectangleV(m.position, (Vector2){30, 30}, PURPLE);
    }
}

static void UpdateOneMonster(Monster *m, Vector2 targetPos, Map *map) {
    
    float distance = Vector2Distance(m->position, targetPos);

    // IA DO ESQUELETO: Só persegue se estiver perto (< 250 pixels)
    if (m->type == MONSTER_SKELETON && distance > 150.0f) {
        return; // Fica parado esperando
    }

    // IA DAS SOMBRAS: Perseguem sempre
    
    // Movimento
    Vector2 oldPos = m->position;
    Vector2 direction = Vector2Subtract(targetPos, m->position);
    direction = Vector2Normalize(direction);
    
    // Move
    m->position.x += direction.x * m->speed;
    m->position.y += direction.y * m->speed;

    // Colisão com Paredes (importante para eles não atravessarem tudo)
    // Ajustamos o ponto de colisão para o centro do monstro (+15)
    Vector2 centerPos = {m->position.x + 15, m->position.y + 15};
    if (CheckMapCollision(*map, centerPos)) {
        m->position = oldPos;
    }
}

void SpawnMonster(Vector2 position, MonsterType type) {
    MonsterNode *novoMonstro = (MonsterNode *)malloc(sizeof(MonsterNode));
    if (novoMonstro == NULL) return;
    
    novoMonstro->data.position = position;
    novoMonstro->data.type = type;
    
    if (type == MONSTER_SKELETON) {
        novoMonstro->data.life = 100;
        novoMonstro->data.speed = 1.0f; 
        novoMonstro->data.color = BEIGE;
        novoMonstro->data.activeRange = 190.0f;
    } else {
        novoMonstro->data.life = 60;
        novoMonstro->data.speed = 2.5f; 
        novoMonstro->data.color = PURPLE;
        novoMonstro->data.activeRange = 2000.0f;
    }
    
    novoMonstro->next = listaDeMonstros;
    listaDeMonstros = novoMonstro;
}

void UpdateMonsters(Player *p1, Player *p2, int numPlayers, Map *map) {
    MonsterNode *current = listaDeMonstros;
    
    while (current != NULL) {
        // Lógica de Alvo (Targeting)
        Vector2 targetPos = p1->position; 

        if (numPlayers == 2) {
            if (p1->ghost && !p2->ghost) targetPos = p2->position;
            else if (!p1->ghost && !p2->ghost) {
                float distP1 = Vector2DistanceSqr(current->data.position, p1->position);
                float distP2 = Vector2DistanceSqr(current->data.position, p2->position);
                if (distP2 < distP1) targetPos = p2->position;
            }
        }
        
        // Adiciona um pequeno "jitter" (tremor) para eles não ficarem perfeitamente empilhados
        // Se estiverem muito perto do alvo, param de andar
        if (Vector2Distance(current->data.position, targetPos) > 10.0f) {
             UpdateOneMonster(&(current->data), targetPos, map);
        }
       
        current = current->next;
    }
}

void DrawMonsters(void) {
    MonsterNode *current = listaDeMonstros;
    while (current != NULL) {
        DrawOneMonster(current->data);
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

int CheckMonsterCollision(Rectangle rect) {
    MonsterNode *prev = NULL;
    MonsterNode *current = listaDeMonstros;

    while (current != NULL) {
        MonsterNode *nextNode = current->next;
        Rectangle monsterRect = {
            .x = current->data.position.x,
            .y = current->data.position.y,
            .width = 30, .height = 30
        };

        if (CheckCollisionRecs(rect, monsterRect)) {
            current->data.life -= 30;
            if (prev == NULL) listaDeMonstros = nextNode;
            else prev->next = nextNode;
            free(current);
            return 100;
        }
        prev = current;
        current = nextNode;
    }
    return 0;
}

// CHECA SE O JOGADOR FOI ATINGIDO
bool CheckPlayerHit(Vector2 playerPosition, float playerRadius) {
    MonsterNode *current = listaDeMonstros;
    while (current != NULL) {
        // Hitbox do Monstro (Quadrado 30x30)
        Rectangle monsterRect = {
            .x = current->data.position.x,
            .y = current->data.position.y,
            .width = 30, .height = 30
        };
        
        // A Raylib checa se o circulo do player encosta no quadrado do monstro
        if (CheckCollisionCircleRec(playerPosition, playerRadius, monsterRect)) {
            return true; // Bateu!
        }
        current = current->next;
    }
    return false;
}

int CheckMeleeAttack(Vector2 playerPos, float range, int damage) {
    int totalScore = 0;
    
    MonsterNode *prev = NULL;
    MonsterNode *current = listaDeMonstros;

    while (current != NULL) {
        MonsterNode *nextNode = current->next;
        
        // Calcula a distância entre o monstro e o jogador
        float dist = Vector2Distance(current->data.position, playerPos);
        
        // Se a distância for menor que o alcance (ex: 60px)
        if (dist <= range) {
            // ACERTOU!
            current->data.life -= damage; // Tira vida
            
            // Se a vida zerou, mata o monstro
            if (current->data.life <= 0) {
                if (prev == NULL) {
                    listaDeMonstros = nextNode;
                } else {
                    prev->next = nextNode;
                }
                free(current);
                totalScore += 100; // Soma pontos
                
                // Importante: Como removemos o nó atual, o 'prev' continua o mesmo
                current = nextNode; 
                continue; // Pula para a próxima iteração
            }
        }
        
        prev = current;
        current = nextNode;
    }
    
    return totalScore;
}

int GetMonsterCount(void) {
    int count = 0;
    MonsterNode *current = listaDeMonstros;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

MonsterNode* GetClosestMonsterNode(Vector2 pos, float range) {
    MonsterNode *closest = NULL;
    float minDist = range; // Só pega se estiver dentro do range

    MonsterNode *current = listaDeMonstros;
    while (current != NULL) {
        float dist = Vector2Distance(current->data.position, pos);
        if (dist < minDist) {
            minDist = dist;
            closest = current;
        }
        current = current->next;
    }
    return closest;
}