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
#define ORC_SCALE 3.0f
#define SLIME_SCALE 2.0f
#define BOSS_SCALE 3.5f
#define DEMON_BOSS_SCALE 2.0f   

typedef struct OrcSprites {
    Texture2D idle, walk, attack, hurt, death;
} OrcSprites;

typedef struct SlimeSprites {
    Texture2D idle, walk, attack, hurt, death;
} SlimeSprites;

static bool orcTexturesLoaded = false;
static OrcSprites orcSprites[ORC_VARIANT_COUNT];

static bool slimeTexturesLoaded = false;
static SlimeSprites slimeSprites;


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

// ORC FRAMES / SPEEDS / STATS 

// ordem dos estados na tabela: IDLE, WALK, ATTACK, HURT, DEATH
static const int orcMaxFrames[ORC_VARIANT_COUNT][5] = {
    [ORC_NORMAL]  = {6, 8, 6, 4, 4},
    [ORC_ARMORED] = {6, 8, 7, 4, 4},
    [ORC_ELITE]   = {6, 8, 9, 4, 4},
};

static const float orcAttackAnimSpeed[ORC_VARIANT_COUNT] = {
    [ORC_NORMAL]  = 0.10f,
    [ORC_ARMORED] = 0.10f,
    [ORC_ELITE]   = 0.14f,  // Elite mais lento
};

// cooldown só pra elite/rider
static const float orcAttackCooldown[ORC_VARIANT_COUNT] = {
    [ORC_NORMAL]  = 0.0f,
    [ORC_ARMORED] = 0.0f,
    [ORC_ELITE]   = 0.8f,   // Elite cooldown
};

// ===== STATS (edite aqui quando quiser) =====
static const int orcLife[ORC_VARIANT_COUNT] = {
    [ORC_NORMAL]  = 65,
    [ORC_ARMORED] = 80,
    [ORC_ELITE]   = 100,
};

static const float orcMoveSpeed[ORC_VARIANT_COUNT] = {
    [ORC_NORMAL]  = 1.0f,
    [ORC_ARMORED] = 1.0f,
    [ORC_ELITE]   = 1.0f,
};

static const int orcDamage[ORC_VARIANT_COUNT] = {
    [ORC_NORMAL]  = 10,
    [ORC_ARMORED] = 10,
    [ORC_ELITE]   = 15,
};

static const float orcActiveRange[ORC_VARIANT_COUNT] = {
    [ORC_NORMAL]  = 200.0f,
    [ORC_ARMORED] = 200.0f,
    [ORC_ELITE]   = 220.0f,
};


// SLIME FRAMES / SPEEDS / STATS

static const int slimeMaxFrames[5] = {
    6,  // IDLE
    6,  // WALK
    12, // ATTACK
    4,  // HURT
    4   // DEATH
};

static const float slimeAttackAnimSpeed = 0.30f;

// STATS
static const int slimeLife = 50;
static const float slimeMoveSpeed = 0.8f; 
static const int slimeDamage = 5;
static const float slimeActiveRange = 180.0f;

typedef struct BossSprites {
    Texture2D idle, walk, attack, hurt, death, transition;
} BossSprites;

static bool bossTexturesLoaded = false;
static BossSprites bossSprites[4]; 
// 0 = Werewolf, 1 = Werebear, 2 = Orc Rider, 3 = Demon

typedef enum {
    BOSS_WEREWOLF = 0,
    BOSS_WEREBEAR,
    BOSS_ORC_RIDER,
    BOSS_DEMON,
    BOSS_COUNT
} BossIndex;

static const int bossMaxFrames[BOSS_COUNT][6] = {
    // idle, walk, attack, hurt, death, transition
    [BOSS_WEREWOLF] = {6, 8, 9, 4, 4, 0},
    [BOSS_WEREBEAR] = {6, 8, 9, 4, 4, 0},
    [BOSS_ORC_RIDER]= {6, 8,11, 4, 4, 0},
    [BOSS_DEMON]    = {4, 4, 6, 3,10, 3}
};
static const float bossAttackAnimSpeed[4] = {
    0.14f,  // Werewolf
    0.14f,  // Werebear
    0.18f,  // Orc Rider boss
    0.16f   // Demon
};

typedef struct BossStats {
    int life;
    int damage;
    float speed;
    float activeRange;
    float attackCooldown;
} BossStats;

// aqui você edita vida/dano/etc dos bosses
static const BossStats bossStats[4] = {
    {250, 20, 1.6f, 220.0f, 0.8f},   // Werewolf
    {320, 25, 1.3f, 200.0f, 1.0f},   // Werebear
    {380, 30, 1.4f, 230.0f, 1.2f},   // Orc Rider boss
    {500, 35, 1.8f, 260.0f, 0.9f}    // Demon
};

static int BossTypeToIndex(MonsterType t) {
    switch (t) {
        case MONSTER_BOSS_WEREWOLF:  return 0;
        case MONSTER_BOSS_WEREBEAR:  return 1;
        case MONSTER_BOSS_ORC_RIDER: return 2;
        case MONSTER_BOSS_DEMON:     return 3;
        default: return -1;
    }
}


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

static void LoadOrcTexturesOnce(void) {
    if (orcTexturesLoaded) return;

    orcSprites[ORC_NORMAL].idle   = LoadTexture("resources/monsters/orc/Orc-Idle.png");
    orcSprites[ORC_NORMAL].walk   = LoadTexture("resources/monsters/orc/Orc-Walk.png");
    orcSprites[ORC_NORMAL].attack = LoadTexture("resources/monsters/orc/Orc-Attack.png");
    orcSprites[ORC_NORMAL].hurt   = LoadTexture("resources/monsters/orc/Orc-Hurt.png");
    orcSprites[ORC_NORMAL].death  = LoadTexture("resources/monsters/orc/Orc-Death.png");

    orcSprites[ORC_ARMORED].idle   = LoadTexture("resources/monsters/armored-orc/Armored Orc-Idle.png");
    orcSprites[ORC_ARMORED].walk   = LoadTexture("resources/monsters/armored-orc/Armored Orc-Walk.png");
    orcSprites[ORC_ARMORED].attack = LoadTexture("resources/monsters/armored-orc/Armored Orc-Attack.png");
    orcSprites[ORC_ARMORED].hurt   = LoadTexture("resources/monsters/armored-orc/Armored Orc-Hurt.png");
    orcSprites[ORC_ARMORED].death  = LoadTexture("resources/monsters/armored-orc/Armored Orc-Death.png");

    orcSprites[ORC_ELITE].idle   = LoadTexture("resources/monsters/elite-orc/Elite Orc-Idle.png");
    orcSprites[ORC_ELITE].walk   = LoadTexture("resources/monsters/elite-orc/Elite Orc-Walk.png");
    orcSprites[ORC_ELITE].attack = LoadTexture("resources/monsters/elite-orc/Elite Orc-Attack.png");
    orcSprites[ORC_ELITE].hurt   = LoadTexture("resources/monsters/elite-orc/Elite Orc-Hurt.png");
    orcSprites[ORC_ELITE].death  = LoadTexture("resources/monsters/elite-orc/Elite Orc-Death.png");

    orcTexturesLoaded = true;
}

static void UnloadOrcTexturesOnce(void) {
    if (!orcTexturesLoaded) return;

    for (int i = 0; i < ORC_VARIANT_COUNT; i++) {
        if (orcSprites[i].idle.id)   UnloadTexture(orcSprites[i].idle);
        if (orcSprites[i].walk.id)   UnloadTexture(orcSprites[i].walk);
        if (orcSprites[i].attack.id) UnloadTexture(orcSprites[i].attack);
        if (orcSprites[i].hurt.id)   UnloadTexture(orcSprites[i].hurt);
        if (orcSprites[i].death.id)  UnloadTexture(orcSprites[i].death);
    }

    orcTexturesLoaded = false;
}

static void LoadSlimeTexturesOnce(void) {
    if (slimeTexturesLoaded) return;

    slimeSprites.idle   = LoadTexture("resources/monsters/slime/Slime-Idle.png");
    slimeSprites.walk   = LoadTexture("resources/monsters/slime/Slime-Walk.png");
    slimeSprites.attack = LoadTexture("resources/monsters/slime/Slime-Attack.png");
    slimeSprites.hurt   = LoadTexture("resources/monsters/slime/Slime-Hurt.png");
    slimeSprites.death  = LoadTexture("resources/monsters/slime/Slime-Death.png");

    slimeTexturesLoaded = true;
}

static void UnloadSlimeTexturesOnce(void) {
    if (!slimeTexturesLoaded) return;

    if (slimeSprites.idle.id)   UnloadTexture(slimeSprites.idle);
    if (slimeSprites.walk.id)   UnloadTexture(slimeSprites.walk);
    if (slimeSprites.attack.id) UnloadTexture(slimeSprites.attack);
    if (slimeSprites.hurt.id)   UnloadTexture(slimeSprites.hurt);
    if (slimeSprites.death.id)  UnloadTexture(slimeSprites.death);

    slimeTexturesLoaded = false;
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

static void LoadBossTexturesOnce(void) {
    if (bossTexturesLoaded) return;

    // Werewolf
    bossSprites[0].idle   = LoadTexture("resources/bosses/werewolf/Werewolf-Idle.png");
    bossSprites[0].walk   = LoadTexture("resources/bosses/werewolf/Werewolf-Walk.png");
    bossSprites[0].attack = LoadTexture("resources/bosses/werewolf/Werewolf-Attack.png");
    bossSprites[0].hurt   = LoadTexture("resources/bosses/werewolf/Werewolf-Hurt.png");
    bossSprites[0].death  = LoadTexture("resources/bosses/werewolf/Werewolf-Death.png");

    // Werebear
    bossSprites[1].idle   = LoadTexture("resources/bosses/werebear/Werebear-Idle.png");
    bossSprites[1].walk   = LoadTexture("resources/bosses/werebear/Werebear-Walk.png");
    bossSprites[1].attack = LoadTexture("resources/bosses/werebear/Werebear-Attack.png");
    bossSprites[1].hurt   = LoadTexture("resources/bosses/werebear/Werebear-Hurt.png");
    bossSprites[1].death  = LoadTexture("resources/bosses/werebear/Werebear-Death.png");

    // Orc Rider (boss)
    bossSprites[2].idle   = LoadTexture("resources/bosses/orc-rider/Orc Rider-Idle.png");
    bossSprites[2].walk   = LoadTexture("resources/bosses/orc-rider/Orc Rider-Walk.png");
    bossSprites[2].attack = LoadTexture("resources/bosses/orc-rider/Orc Rider-Attack.png");
    bossSprites[2].hurt   = LoadTexture("resources/bosses/orc-rider/Orc Rider-Hurt.png");
    bossSprites[2].death  = LoadTexture("resources/bosses/orc-rider/Orc Rider-Death.png");

    // Demon – WALK usa o Flying
    bossSprites[3].idle   = LoadTexture("resources/bosses/demon/Demon-Idle.png");
    bossSprites[3].walk   = LoadTexture("resources/bosses/demon/Demon-Flying.png");
    bossSprites[3].attack = LoadTexture("resources/bosses/demon/Demon-Attack.png");
    bossSprites[3].hurt   = LoadTexture("resources/bosses/demon/Demon-Hurt.png");
    bossSprites[3].death  = LoadTexture("resources/bosses/demon/Demon-Death.png");
    bossSprites[3].transition = LoadTexture("resources/bosses/demon/Demon-Transition.png"); 

    bossTexturesLoaded = true;
}

static void UnloadBossTexturesOnce(void) {
    if (!bossTexturesLoaded) return;

    for (int i = 0; i < 4; i++) {
        if (bossSprites[i].idle.id)   UnloadTexture(bossSprites[i].idle);
        if (bossSprites[i].walk.id)   UnloadTexture(bossSprites[i].walk);
        if (bossSprites[i].attack.id) UnloadTexture(bossSprites[i].attack);
        if (bossSprites[i].hurt.id)   UnloadTexture(bossSprites[i].hurt);
        if (bossSprites[i].death.id)  UnloadTexture(bossSprites[i].death);
    }

    bossTexturesLoaded = false;
}



int GetMonsterMaxFramesGeneric(Monster *m, MonsterAnimState st) {
    // ignora TRANSITION para os outros monstros
    if (st < MONSTER_IDLE || st > MONSTER_DEATH)
        st = MONSTER_IDLE;

    if (m->type == MONSTER_SKELETON) {
        return GetSkeletonMaxFrames(m->skelVariant, st);
    }
    if (m->type == MONSTER_ORC) {
        OrcVariant v = m->orcVariant;
        if (v < 0 || v >= ORC_VARIANT_COUNT) v = ORC_NORMAL;
        return orcMaxFrames[v][st];
    }
    if (m->type == MONSTER_SLIME) {
        return slimeMaxFrames[st];
    }

    // ====== BOSSES ======
    int bossIdx = BossTypeToIndex(m->type);
    if (bossIdx >= 0) {
        int col = (int)st;
        if (st == MONSTER_TRANSITION) col = 5; //  transition
        if (col < 0 || col > 5) col = 0;
        return bossMaxFrames[bossIdx][col];
    }

    return 1;
}

float GetMonsterAttackAnimSpeedGeneric(Monster *m) {

    int idx = BossTypeToIndex(m->type);
    if (idx >= 0) {
        return bossAttackAnimSpeed[idx];
    }

    if (m->type == MONSTER_SKELETON) {
        return GetSkeletonAttackAnimSpeed(m->skelVariant);
    }
    if (m->type == MONSTER_ORC) {
        OrcVariant v = m->orcVariant;
        if (v < 0 || v >= ORC_VARIANT_COUNT) v = ORC_NORMAL;
        return orcAttackAnimSpeed[v];
    }
    if (m->type == MONSTER_SLIME) {
        return slimeAttackAnimSpeed;
    }
    return MONSTER_ATTACK_SPEED;
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
        data->monsters[count].skelVariant = (int)current->data.skelVariant;
        data->monsters[count].orcVariant  = (int)current->data.orcVariant;

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
        listaDeMonstros->data.skelVariant = (SkeletonVariant)data.monsters[i].skelVariant;
        listaDeMonstros->data.orcVariant  = (OrcVariant)data.monsters[i].orcVariant;

        // re-aponta sprites corretos
        if (listaDeMonstros->data.type == MONSTER_SKELETON) {
            SkeletonVariant v = listaDeMonstros->data.skelVariant;
            if (v < 0 || v >= SKEL_VARIANT_COUNT) v = SKEL_NORMAL;
            listaDeMonstros->data.texIdle   = skSprites[v].idle;
            listaDeMonstros->data.texWalk   = skSprites[v].walk;
            listaDeMonstros->data.texAttack = skSprites[v].attack;
            listaDeMonstros->data.texHurt   = skSprites[v].hurt;
            listaDeMonstros->data.texDeath  = skSprites[v].death;
        }
        else if (listaDeMonstros->data.type == MONSTER_ORC) {
            OrcVariant v = listaDeMonstros->data.orcVariant;
            if (v < 0 || v >= ORC_VARIANT_COUNT) v = ORC_NORMAL;
            listaDeMonstros->data.texIdle   = orcSprites[v].idle;
            listaDeMonstros->data.texWalk   = orcSprites[v].walk;
            listaDeMonstros->data.texAttack = orcSprites[v].attack;
            listaDeMonstros->data.texHurt   = orcSprites[v].hurt;
            listaDeMonstros->data.texDeath  = orcSprites[v].death;
        }
        else if (listaDeMonstros->data.type == MONSTER_SLIME) {
            listaDeMonstros->data.texIdle   = slimeSprites.idle;
            listaDeMonstros->data.texWalk   = slimeSprites.walk;
            listaDeMonstros->data.texAttack = slimeSprites.attack;
            listaDeMonstros->data.texHurt   = slimeSprites.hurt;
            listaDeMonstros->data.texDeath  = slimeSprites.death;
        }

    }
}

// Desenho melhorado para identificar quem é quem
static void DrawOneMonster(Monster m) {

    bool hasSprite = (m.texIdle.id != 0);


    if (!hasSprite) {
        DrawRectangleV(m.position, (Vector2){30, 30}, (m.type==MONSTER_SLIME? GREEN : BEIGE));
        DrawRectangleLines(m.position.x, m.position.y, 30, 30, BLACK);
        return;
    }

    Texture2D texToDraw = m.texIdle;
    switch (m.state) {
        case MONSTER_IDLE:   texToDraw = m.texIdle;   break;
        case MONSTER_WALK:   texToDraw = m.texWalk;   break;
        case MONSTER_ATTACK: texToDraw = m.texAttack; break;
        case MONSTER_HURT:   texToDraw = m.texHurt;   break;
        case MONSTER_DEATH:  texToDraw = m.texDeath;  break;
        case MONSTER_TRANSITION: 
        if (m.type == MONSTER_BOSS_DEMON && m.texTransition.id != 0)
            texToDraw = m.texTransition;
        else
            texToDraw = m.texWalk; // fallback
        break;
        default: break;
    }

    int maxFrames = GetMonsterMaxFramesGeneric(&m, m.state);
    float frameWidth = (float)texToDraw.width / maxFrames;

    Rectangle sourceRec = {
        frameWidth * m.currentFrame,
        0.0f,
        frameWidth * m.facingDirection,
        (float)texToDraw.height
    };

    Vector2 center = { m.position.x + 15, m.position.y + 15 };

    float scale = SKELETON_SCALE;

    if (m.type == MONSTER_ORC) {
        scale = ORC_SCALE;
    }
    else if (m.type == MONSTER_SLIME) {
        scale = SLIME_SCALE;
    }
    else if (BossTypeToIndex(m.type) >= 0) { 
        if (m.type == MONSTER_BOSS_DEMON) {
            scale = DEMON_BOSS_SCALE;   // demon
        } else {
            scale = BOSS_SCALE;         // werewolf, werebear, orc rider
        }
    }



    Rectangle destRec = {
        center.x,
        center.y,
        frameWidth * scale,
        (float)texToDraw.height * scale
    };

    Vector2 origin = { destRec.width/2.0f, destRec.height/2.0f };

    Color tint = (m.color.a == 0 ? WHITE : m.color);
    DrawTexturePro(texToDraw, sourceRec, destRec, origin, 0.0f, tint);
}



static void UpdateOneMonster(Monster *m, Vector2 targetPos, Vector2 separationForce, Map *map) {
    Vector2 monsterCenter = { m->position.x + 15, m->position.y + 15 };
    float distance = Vector2Distance(monsterCenter, targetPos);

    bool isBoss = (
        m->type == MONSTER_BOSS_WEREWOLF  ||
        m->type == MONSTER_BOSS_WEREBEAR  ||
        m->type == MONSTER_BOSS_ORC_RIDER ||
        m->type == MONSTER_BOSS_DEMON
    );

    bool isMeleeAnimated = (
        m->type == MONSTER_SKELETON ||
        m->type == MONSTER_ORC      ||
        m->type == MONSTER_SLIME    ||
        isBoss
    );

    bool isDemon = (m->type == MONSTER_BOSS_DEMON);

    // baixa cooldown sempre
    if (m->attackCooldownTimer > 0) {
        m->attackCooldownTimer -= GetFrameTime();
        if (m->attackCooldownTimer < 0) m->attackCooldownTimer = 0;
    }

    bool canMove = true;
    if (isMeleeAnimated) {
        if (m->state == MONSTER_DEATH) canMove = false;
        if (m->state == MONSTER_HURT)  canMove = false;
        if (distance > m->activeRange) canMove = false;
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
    if (isMeleeAnimated) {

        if (finalDir.x < -0.01f) m->facingDirection = -1;
        else if (finalDir.x > 0.01f) m->facingDirection = 1;

        if (m->state != MONSTER_HURT && m->state != MONSTER_DEATH) {

            if (distance <= 35.0f && m->attackCooldownTimer <= 0.0f) {
                m->state = MONSTER_ATTACK;
            }
            else if (canMove) {
                if (isDemon && m->state == MONSTER_IDLE) {
                    // toca a transition
                    m->state        = MONSTER_TRANSITION;
                    m->currentFrame = 0;
                    m->frameTime    = 0.0f;
                } else if (m->state != MONSTER_TRANSITION) {
                    // qualquer outro monster ou demon já transicionado
                    m->state = MONSTER_WALK;
                }
            }
            else {
                m->state = MONSTER_IDLE;
            }
        }


        float animSpeed = MONSTER_ANIM_SPEED;
        if (m->state == MONSTER_ATTACK) animSpeed = GetMonsterAttackAnimSpeedGeneric(m);
        if (m->state == MONSTER_HURT)   animSpeed = 0.08f;
        if (m->state == MONSTER_DEATH)  animSpeed = 0.12f;
        if (m->state == MONSTER_TRANSITION) animSpeed = 0.10f;


        m->frameTime += GetFrameTime();
        if (m->frameTime >= animSpeed) {
            m->frameTime = 0.0f;
            m->currentFrame++;

            int maxF = GetMonsterMaxFramesGeneric(m, m->state);

            if (m->currentFrame >= maxF) {
                if (m->state == MONSTER_HURT) {
                    m->state = MONSTER_WALK;
                    m->currentFrame = 0;
                }
                else if (m->state == MONSTER_DEATH) {
                    m->currentFrame = maxF - 1;
                }
                else if (m->state == MONSTER_ATTACK) {
                    m->currentFrame = 0;
                    // aplica cooldown quando termina o ataque
                    m->attackCooldownTimer = m->attackCooldown;
                    m->state = MONSTER_IDLE;
                }
                else if (m->state == MONSTER_TRANSITION && isDemon) {
                    m->state = MONSTER_WALK;
                    m->currentFrame = 0;
                }
                else {
                    m->currentFrame = 0;
                }
            }
        }
    }

    
}


static OrcVariant RollOrcVariantWeighted(void) {
    // Rider mais raro
    int roll = GetRandomValue(0, 99);
    if (roll < 55) return ORC_NORMAL;      // 55%
    if (roll < 85) return ORC_ARMORED;     // 30% 
    return ORC_ELITE; // 15%
}

void SpawnMonster(Vector2 position, MonsterType type) {
    MonsterNode *novoMonstro = (MonsterNode *)malloc(sizeof(MonsterNode));
    if (novoMonstro == NULL) return;

    Monster *m = &novoMonstro->data;
    m->position = position;
    m->type = type;

    m->isPossessed = false;

    // defaults
    m->state = MONSTER_IDLE;
    m->currentFrame = 0;
    m->frameTime = 0.0f;
    m->facingDirection = 1;

    m->attackCooldown = 0.0f;
    m->attackCooldownTimer = 0.0f;

    int bossIdx = BossTypeToIndex(type);
    if (bossIdx >= 0) {
        LoadBossTexturesOnce();

        m->texIdle   = bossSprites[bossIdx].idle;
        m->texWalk   = bossSprites[bossIdx].walk;
        m->texAttack = bossSprites[bossIdx].attack;
        m->texHurt   = bossSprites[bossIdx].hurt;
        m->texDeath  = bossSprites[bossIdx].death;
        m->texTransition = bossSprites[bossIdx].transition;

        BossStats st = bossStats[bossIdx];
        m->life            = st.life;
        m->damage          = st.damage;
        m->speed           = st.speed;
        m->activeRange     = st.activeRange;
        m->attackCooldown  = st.attackCooldown;
        m->color           = WHITE;

        novoMonstro->next = listaDeMonstros;
        listaDeMonstros   = novoMonstro;
        return;
    }

    if (type == MONSTER_SKELETON) {
        LoadSkeletonTexturesOnce();

        SkeletonVariant v = (SkeletonVariant)GetRandomValue(0, SKEL_VARIANT_COUNT - 1);
        m->skelVariant = v;

        m->texIdle   = skSprites[v].idle;
        m->texWalk   = skSprites[v].walk;
        m->texAttack = skSprites[v].attack;
        m->texHurt   = skSprites[v].hurt;
        m->texDeath  = skSprites[v].death;

        // ===== SKELETON STATS =====
        m->life = 70;
        m->damage = 15;
        m->speed = 1.0f;
        m->activeRange = 190.0f;
        m->attackCooldown = 0.0f;

        m->color = WHITE;

    }
    else if (type == MONSTER_ORC) {
        LoadOrcTexturesOnce();

        OrcVariant v = RollOrcVariantWeighted();
        m->orcVariant = v;

        m->texIdle   = orcSprites[v].idle;
        m->texWalk   = orcSprites[v].walk;
        m->texAttack = orcSprites[v].attack;
        m->texHurt   = orcSprites[v].hurt;
        m->texDeath  = orcSprites[v].death;

        // ===== ORC STATS =====
        m->life = orcLife[v];
        m->damage = orcDamage[v];
        m->speed = orcMoveSpeed[v];
        m->activeRange = orcActiveRange[v];
        m->attackCooldown = orcAttackCooldown[v];

        m->color = WHITE;
    }
    else if (type == MONSTER_SLIME) {
        LoadSlimeTexturesOnce();

        m->texIdle   = slimeSprites.idle;
        m->texWalk   = slimeSprites.walk;
        m->texAttack = slimeSprites.attack;
        m->texHurt   = slimeSprites.hurt;
        m->texDeath  = slimeSprites.death;

        // ===== SLIME STATS =====
        m->life = slimeLife;
        m->damage = slimeDamage;
        m->speed = slimeMoveSpeed;
        m->activeRange = slimeActiveRange;
        m->attackCooldown = 0.0f;

        m->color = WHITE;
    }
    else {
        // shadows placeholder
        m->life = 60;
        m->damage = 20;
        m->speed = 2.5f;
        m->color = PURPLE;
        m->activeRange = 2000.0f;

        m->texIdle.id = 0;
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

        bool isAnimatedType = (current->data.type == MONSTER_SKELETON || current->data.type == MONSTER_ORC || current->data.type == MONSTER_SLIME || BossTypeToIndex(current->data.type) >= 0);

        if (!possessedByP1 && !possessedByP2 &&
            isAnimatedType &&
            current->data.state == MONSTER_DEATH) {

            int maxF = GetMonsterMaxFramesGeneric(&current->data, MONSTER_DEATH);
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
    UnloadOrcTexturesOnce();
    UnloadSlimeTexturesOnce();
    UnloadBossTexturesOnce();


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
            bool isAnimatedType =
            (current->data.type == MONSTER_SKELETON
            || current->data.type == MONSTER_ORC
            || current->data.type == MONSTER_ARMORED_ORC
            || current->data.type == MONSTER_ELITE_ORC
            || current->data.type == MONSTER_ORC_RIDER
            || current->data.type == MONSTER_SLIME
            || BossTypeToIndex(current->data.type) >= 0
            );
            
            // se já está morrendo, não toma dano de novo
            if (isAnimatedType && current->data.state == MONSTER_DEATH) {
                return 0;
            }

            current->data.life -= 30;

            if (isAnimatedType) {
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
            bool isAnimatedType =
            (current->data.type == MONSTER_SKELETON
            || current->data.type == MONSTER_ORC
            || current->data.type == MONSTER_ARMORED_ORC
            || current->data.type == MONSTER_ELITE_ORC
            || current->data.type == MONSTER_ORC_RIDER
            || current->data.type == MONSTER_SLIME
            || BossTypeToIndex(current->data.type) >= 0
            );
            // se já está morrendo, ignora
            if (isAnimatedType && current->data.state == MONSTER_DEATH) {
                prev = current;
                current = nextNode;
                continue;
            }

            current->data.life -= damage;

            if (isAnimatedType) {
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