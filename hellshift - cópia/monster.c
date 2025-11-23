#include "monster.h"
#include <stdlib.h>
#include "raymath.h" 
#include "player.h"
#include "map.h"
#include "save.h"

static MonsterNode *listaDeMonstros = NULL;

void ExportMonsters(SaveData *data) {
    int count = 0;
    MonsterNode *current = listaDeMonstros;
    
    // Percorre a lista e copia cada um para o array do SaveData
    while (current != NULL && count < MAX_SAVED_MONSTERS) {
        data->monsters[count].position = current->data.position;
        data->monsters[count].type = (int)current->data.type;
        data->monsters[count].life = current->data.life;
        data->monsters[count].color = current->data.color;
        data->monsters[count].speed = current->data.speed;
        data->monsters[count].activeRange = current->data.activeRange;
        
        count++;
        current = current->next;
    }
    data->monsterCount = count;
}

void ImportMonsters(SaveData data) {
    UnloadMonsters(); // Limpa monstros existentes
    
    // Recria a lista a partir do array salvo
    for (int i = 0; i < data.monsterCount; i++) {
        SpawnMonster(data.monsters[i].position, (MonsterType)data.monsters[i].type);
        
        // Sobrescreve status base pelos salvos
        if (listaDeMonstros != NULL) {
            listaDeMonstros->data.life = data.monsters[i].life;
            listaDeMonstros->data.color = data.monsters[i].color;
            listaDeMonstros->data.speed = data.monsters[i].speed;
            listaDeMonstros->data.activeRange = data.monsters[i].activeRange;
        }
    }
}

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

static void UpdateOneMonster(Monster *m, Vector2 targetPos, Vector2 separationForce, Map *map) {
    
    Vector2 monsterCenter = { m->position.x + 15, m->position.y + 15 };
    float distance = Vector2Distance(monsterCenter, targetPos);

    // IA DO ESQUELETO: Distância de ativação
    if (m->type == MONSTER_SKELETON && distance > 250.0f) {
        return; 
    }

    Vector2 oldPos = m->position;
    
    // vetor de Perseguição
    Vector2 chaseDir = Vector2Subtract(targetPos, monsterCenter);
    chaseDir = Vector2Normalize(chaseDir);

    Vector2 finalDir = Vector2Add(chaseDir, Vector2Scale(separationForce, 1.5f));
    
    finalDir = Vector2Normalize(finalDir);

    // Move
    m->position.x += finalDir.x * m->speed;
    m->position.y += finalDir.y * m->speed;

    // Colisão com Paredes
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
        //possessão player
        if (p1->isPossessing && (MonsterNode*)p1->possessedMonster == current) {
            current = current->next;
            continue; // O Player.c cuida do movimento deste aqui
        }
        if (numPlayers == 2 && p2->isPossessing && (MonsterNode*)p2->possessedMonster == current) {
            current = current->next;
            continue;
        }
        // alvo
        Vector2 targetPos = p1->position; 
        if (numPlayers == 2) {
            if (p1->ghost && !p2->ghost) targetPos = p2->position;
            else if (!p1->ghost && !p2->ghost) {
                float distP1 = Vector2DistanceSqr(current->data.position, p1->position);
                float distP2 = Vector2DistanceSqr(current->data.position, p2->position);
                if (distP2 < distP1) targetPos = p2->position;
            }
        }
        
        // Anti-Stacking
        Vector2 separation = { 0.0f, 0.0f };
        MonsterNode *other = listaDeMonstros;
        
        while (other != NULL) {
            if (current != other) { // Não comparar com ele mesmo
                float dist = Vector2Distance(current->data.position, other->data.position);
                
                // Se estiver muito perto (menos de 25 pixels), empurra!
                if (dist < 25.0f) {
                    Vector2 push = Vector2Subtract(current->data.position, other->data.position);
                    push = Vector2Normalize(push);
                    separation = Vector2Add(separation, push);
                }
            }
            other = other->next;
        }

        // atualiza enviando a força de separação
        UpdateOneMonster(&(current->data), targetPos, separation, map);
       
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
        // O monstro tem 30x30. O centro dele é +15,+15.
        Vector2 monsterCenter = { 
            current->data.position.x + 15, 
            current->data.position.y + 15 
        };
        
        // raio do monstro
        float monsterRadius = 15.0f;

        // distância real
        float distance = Vector2Distance(playerPosition, monsterCenter);
        
    
        if (distance < (playerRadius + monsterRadius)) {
            return true; // dano
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