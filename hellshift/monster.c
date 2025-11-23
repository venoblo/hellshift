#include "monster.h"
#include <stdlib.h>
#include "raymath.h" 
#include "player.h"
#include "map.h"
#include "save.h"

static MonsterNode *listaDeMonstros = NULL;

#define SKELETON_SCALE 3.0f
#define MONSTER_ANIM_SPEED 0.15f
#define MONSTER_ATTACK_SPEED 0.10f

typedef struct SkeletonSprites {
    Texture2D idle, walk, attack, hurt, death;
} SkeletonSprites;

static bool skeletonTexturesLoaded = false;
static SkeletonSprites skSprites[SKEL_VARIANT_COUNT];

static const float skAttackAnimSpeed[SKEL_VARIANT_COUNT] = {
    [SKEL_NORMAL] = 0.10f,
    [SKEL_ARMORED] = 0.10f,
    [SKEL_GREATSWORD] = 0.16f  // greatsword = ataque mais demorado
};


static const int skMaxFrames[SKEL_VARIANT_COUNT][5] = {
    // SKEL_NORMAL
    [SKEL_NORMAL] = {
        6,  // IDLE
        8,  // WALK
        7,  // ATTACK
        4,  // HURT
        4   // DEATH
    },
    // SKEL_ARMORED
    [SKEL_ARMORED] = {
        6,  // IDLE
        8,  // WALK
        9,  // ATTACK
        4,  // HURT
        4   // DEATH
    },
    // SKEL_GREATSWORD
    [SKEL_GREATSWORD] = {
        6,  // IDLE
        9,  // WALK
        12, // ATTACK
        4,  // HURT
        4   // DEATH
    }
};

int GetSkeletonMaxFrames(SkeletonVariant v, MonsterAnimState st) {
    if (v < 0 || v >= SKEL_VARIANT_COUNT) v = SKEL_NORMAL;
    if (st < 0 || st > MONSTER_DEATH) st = MONSTER_IDLE;
    return skMaxFrames[v][st];
}


float GetSkeletonAttackAnimSpeed(SkeletonVariant v) {
    if (v < 0 || v >= SKEL_VARIANT_COUNT) return 0.10f;
    return skAttackAnimSpeed[v];
}


static void LoadSkeletonTexturesOnce(void) {
    if (skeletonTexturesLoaded) return;

    // NORMAL (já existe)
    skSprites[SKEL_NORMAL].idle   = LoadTexture("resources/monsters/skeleton/Skeleton-Idle.png");
    skSprites[SKEL_NORMAL].walk   = LoadTexture("resources/monsters/skeleton/Skeleton-Walk.png");
    skSprites[SKEL_NORMAL].attack = LoadTexture("resources/monsters/skeleton/Skeleton-Attack.png");
    skSprites[SKEL_NORMAL].hurt   = LoadTexture("resources/monsters/skeleton/Skeleton-Hurt.png");
    skSprites[SKEL_NORMAL].death  = LoadTexture("resources/monsters/skeleton/Skeleton-Death.png");

    // ARMORED
    skSprites[SKEL_ARMORED].idle   = LoadTexture("resources/monsters/armored-skeleton/Armored Skeleton-Idle.png");
    skSprites[SKEL_ARMORED].walk   = LoadTexture("resources/monsters/armored-skeleton/Armored Skeleton-Walk.png");
    skSprites[SKEL_ARMORED].attack = LoadTexture("resources/monsters/armored-skeleton/Armored Skeleton-Attack.png");
    skSprites[SKEL_ARMORED].hurt   = LoadTexture("resources/monsters/armored-skeleton/Armored Skeleton-Hurt.png");
    skSprites[SKEL_ARMORED].death  = LoadTexture("resources/monsters/armored-skeleton/Armored Skeleton-Death.png");

    // GREATSWORD
    skSprites[SKEL_GREATSWORD].idle   = LoadTexture("resources/monsters/greatsword-skeleton/Greatsword Skeleton-Idle.png");
    skSprites[SKEL_GREATSWORD].walk   = LoadTexture("resources/monsters/greatsword-skeleton/Greatsword Skeleton-Walk.png");
    skSprites[SKEL_GREATSWORD].attack = LoadTexture("resources/monsters/greatsword-skeleton/Greatsword Skeleton-Attack.png");
    skSprites[SKEL_GREATSWORD].hurt   = LoadTexture("resources/monsters/greatsword-skeleton/Greatsword Skeleton-Hurt.png");
    skSprites[SKEL_GREATSWORD].death  = LoadTexture("resources/monsters/greatsword-skeleton/Greatsword Skeleton-Death.png");

    skeletonTexturesLoaded = true;
}


static void UnloadSkeletonTexturesOnce(void) {
    if (!skeletonTexturesLoaded) return;

    for (int i = 0; i < SKEL_VARIANT_COUNT; i++) {
        if (skSprites[i].idle.id)   UnloadTexture(skSprites[i].idle);
        if (skSprites[i].walk.id)   UnloadTexture(skSprites[i].walk);
        if (skSprites[i].attack.id) UnloadTexture(skSprites[i].attack);
        if (skSprites[i].hurt.id)   UnloadTexture(skSprites[i].hurt);
        if (skSprites[i].death.id)  UnloadTexture(skSprites[i].death);
    }

    skeletonTexturesLoaded = false;
}


static int GetMonsterMaxFrames(Monster *m) {
    if (m->type == MONSTER_SKELETON) {
        return GetSkeletonMaxFrames(m->skelVariant, m->state);
    }
    return 1;
}


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
    if (m.type != MONSTER_SKELETON || !skeletonTexturesLoaded) {
        // placeholders antigos
        if (m.type == MONSTER_SKELETON) {
            DrawRectangleV(m.position, (Vector2){30, 30}, BEIGE);
            DrawRectangleLines(m.position.x, m.position.y, 30, 30, BLACK);
        } else {
            DrawRectangleV(m.position, (Vector2){30, 30}, PURPLE);
        }
        return;
    }

    Texture2D texToDraw = m.texIdle;
    switch (m.state) {
        case MONSTER_IDLE:   texToDraw = m.texIdle;   break;
        case MONSTER_WALK:   texToDraw = m.texWalk;   break;
        case MONSTER_ATTACK: texToDraw = m.texAttack; break;
        case MONSTER_HURT:   texToDraw = m.texHurt;   break;
        case MONSTER_DEATH:  texToDraw = m.texDeath;  break;
        default: texToDraw = m.texIdle; break;
    }

    int maxFrames = GetMonsterMaxFrames(&m);
    float frameWidth = (float)texToDraw.width / maxFrames;

    Rectangle sourceRec = {
        frameWidth * m.currentFrame,
        0.0f,
        frameWidth * m.facingDirection,
        (float)texToDraw.height
    };

    Vector2 monsterCenter = { m.position.x + 15, m.position.y + 15 };

    Rectangle destRec = {
        monsterCenter.x,
        monsterCenter.y,
        frameWidth * SKELETON_SCALE,
        (float)texToDraw.height * SKELETON_SCALE
    };

    Vector2 origin = { destRec.width/2.0f, destRec.height/2.0f };

    Color tint = m.color;
    if (tint.a == 0) tint = WHITE; // segurança contra lixo/bug de save
    DrawTexturePro(texToDraw, sourceRec, destRec, origin, 0.0f, tint);

}


static void UpdateOneMonster(Monster *m, Vector2 targetPos, Vector2 separationForce, Map *map) {
    Vector2 monsterCenter = { m->position.x + 15, m->position.y + 15 };
    float distance = Vector2Distance(monsterCenter, targetPos);

    bool isSkeleton = (m->type == MONSTER_SKELETON);

    // Decide se pode mover
    bool canMove = true;
    if (isSkeleton) {
        if (m->state == MONSTER_DEATH) canMove = false;
        if (m->state == MONSTER_HURT)  canMove = false;   // fica travado apanhando
        if (distance > 250.0f)         canMove = false;   // range de ativação
    }

    Vector2 oldPos = m->position;
    Vector2 finalDir = {0};

    if (canMove) {
        Vector2 chaseDir = Vector2Subtract(targetPos, monsterCenter);
        chaseDir = Vector2Normalize(chaseDir);

        finalDir = Vector2Add(chaseDir, Vector2Scale(separationForce, 1.5f));
        finalDir = Vector2Normalize(finalDir);

        m->position.x += finalDir.x * m->speed;
        m->position.y += finalDir.y * m->speed;

        Vector2 centerPos = {m->position.x + 15, m->position.y + 15};
        if (CheckMapCollision(*map, centerPos)) {
            m->position = oldPos;
        }
    }

    // --- lógica de estado + animação (roda sempre) ---
    if (isSkeleton) {

        if (finalDir.x < -0.01f) m->facingDirection = -1;
        else if (finalDir.x > 0.01f) m->facingDirection = 1;

        // só muda pra walk/attack se não estiver hurt/death
        if (m->state != MONSTER_HURT && m->state != MONSTER_DEATH) {
            if (distance <= 35.0f) m->state = MONSTER_ATTACK;
            else if (canMove)      m->state = MONSTER_WALK;
            else                   m->state = MONSTER_IDLE;
        }

        float animSpeed = MONSTER_ANIM_SPEED;
        if (m->state == MONSTER_ATTACK) animSpeed = GetSkeletonAttackAnimSpeed(m->skelVariant);
        if (m->state == MONSTER_HURT)   animSpeed = 0.08f;
        if (m->state == MONSTER_DEATH)  animSpeed = 0.12f;

        m->frameTime += GetFrameTime();
        if (m->frameTime >= animSpeed) {
            m->frameTime = 0.0f;
            m->currentFrame++;

            int maxF = GetMonsterMaxFrames(m);

            if (m->currentFrame >= maxF) {
                if (m->state == MONSTER_HURT) {
                    m->state = MONSTER_WALK;
                    m->currentFrame = 0;
                }
                else if (m->state == MONSTER_DEATH) {
                    m->currentFrame = maxF - 1; // trava no último
                }
                else {
                    m->currentFrame = 0;
                }
            }
        }
    }
}


void SpawnMonster(Vector2 position, MonsterType type) {
    MonsterNode *novoMonstro = (MonsterNode *)malloc(sizeof(MonsterNode));
    if (novoMonstro == NULL) return;
    
    novoMonstro->data.position = position;
    novoMonstro->data.type = type;

    novoMonstro->data.isPossessed = false;

    // defaults comuns
    novoMonstro->data.state = MONSTER_IDLE;
    novoMonstro->data.currentFrame = 0;
    novoMonstro->data.frameTime = 0.0f;
    novoMonstro->data.facingDirection = 1;

    if (type == MONSTER_SKELETON) {
        LoadSkeletonTexturesOnce();

        SkeletonVariant v = (SkeletonVariant)GetRandomValue(0, SKEL_VARIANT_COUNT - 1);
        novoMonstro->data.skelVariant = v;

        novoMonstro->data.texIdle   = skSprites[v].idle;
        novoMonstro->data.texWalk   = skSprites[v].walk;
        novoMonstro->data.texAttack = skSprites[v].attack;
        novoMonstro->data.texHurt   = skSprites[v].hurt;
        novoMonstro->data.texDeath  = skSprites[v].death;

        // STATS + COR NORMAIS
        novoMonstro->data.life = 100;
        novoMonstro->data.speed = 1.0f;
        novoMonstro->data.color = WHITE;     // <--- ESSENCIAL
        novoMonstro->data.activeRange = 190.0f;

    } else {
        // placeholder shadows
        novoMonstro->data.life = 60;
        novoMonstro->data.speed = 2.5f;
        novoMonstro->data.color = PURPLE;    // <--- ESSENCIAL
        novoMonstro->data.activeRange = 2000.0f;

        novoMonstro->data.texIdle.id = 0; // resto 0
    }
    
    novoMonstro->next = listaDeMonstros;
    listaDeMonstros = novoMonstro;
}


void UpdateMonsters(Player *p1, Player *p2, int numPlayers, Map *map) {
    MonsterNode *current = listaDeMonstros;
    MonsterNode *prev = NULL;

    while (current != NULL) {
        MonsterNode *next = current->next;

        bool possessedByP1 = (p1->isPossessing && (MonsterNode*)p1->possessedMonster == current);
        bool possessedByP2 = (numPlayers == 2 && p2->isPossessing && (MonsterNode*)p2->possessedMonster == current);

        // remove skeleton só depois do death terminar e se não estiver possuído
        if (!possessedByP1 && !possessedByP2 &&
            current->data.type == MONSTER_SKELETON &&
            current->data.state == MONSTER_DEATH) {

            int maxF = GetMonsterMaxFrames(&current->data);
            if (current->data.currentFrame >= maxF - 1) {
                if (prev == NULL) listaDeMonstros = next;
                else prev->next = next;
                free(current);
                current = next;
                continue;
            }
        }

        // se estiver possuído, Player.c move ele
        if (possessedByP1 || possessedByP2) {
            prev = current;
            current = next;
            continue;
        }

        // --- alvo (igual ao teu código) ---
        Vector2 targetPos = p1->position;
        if (numPlayers == 2) {
            if (p1->ghost && !p2->ghost) targetPos = p2->position;
            else if (!p1->ghost && !p2->ghost) {
                float distP1 = Vector2DistanceSqr(current->data.position, p1->position);
                float distP2 = Vector2DistanceSqr(current->data.position, p2->position);
                if (distP2 < distP1) targetPos = p2->position;
            }
        }

        // --- anti-stacking (igual) ---
        Vector2 separation = {0};
        MonsterNode *other = listaDeMonstros;
        while (other != NULL) {
            if (current != other) {
                float dist = Vector2Distance(current->data.position, other->data.position);
                if (dist < 25.0f) {
                    Vector2 push = Vector2Subtract(current->data.position, other->data.position);
                    push = Vector2Normalize(push);
                    separation = Vector2Add(separation, push);
                }
            }
            other = other->next;
        }

        UpdateOneMonster(&(current->data), targetPos, separation, map);

        prev = current;
        current = next;
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
    UnloadSkeletonTexturesOnce();
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

            // se já está morrendo, não toma dano de novo
            if (current->data.type == MONSTER_SKELETON &&
                current->data.state == MONSTER_DEATH) {
                return 0;
            }

            current->data.life -= 30;

            if (current->data.type == MONSTER_SKELETON) {
                if (current->data.life > 0) {
                    current->data.state = MONSTER_HURT;
                    current->data.currentFrame = 0;
                    current->data.frameTime = 0.0f;
                } else {
                    current->data.life = 0;
                    current->data.state = MONSTER_DEATH;
                    current->data.currentFrame = 0;
                    current->data.frameTime = 0.0f;
                }
                return 100;
            }

            // outros monstros: remove instantâneo
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

        if (current->data.isPossessed) {  
            current = current->next;
            continue;
        }

        if (current->data.type == MONSTER_SKELETON &&
            current->data.state == MONSTER_DEATH) { // opcional mas bom
            current = current->next;
            continue;
        }
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

            // se já está morrendo, ignora
            if (current->data.type == MONSTER_SKELETON &&
                current->data.state == MONSTER_DEATH) {
                prev = current;
                current = nextNode;
                continue;
            }

            current->data.life -= damage;

            if (current->data.type == MONSTER_SKELETON) {
                if (current->data.life > 0) {
                    current->data.state = MONSTER_HURT;
                    current->data.currentFrame = 0;
                    current->data.frameTime = 0.0f;
                } else {
                    current->data.life = 0;
                    current->data.state = MONSTER_DEATH;
                    current->data.currentFrame = 0;
                    current->data.frameTime = 0.0f;
                    totalScore += 100;
                }

                prev = current;
                current = nextNode;
                continue;
            }

            // outros monstros: remove na hora
            if (current->data.life <= 0) {
                if (prev == NULL) listaDeMonstros = nextNode;
                else prev->next = nextNode;
                free(current);
                totalScore += 100;
                current = nextNode;
                continue;
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