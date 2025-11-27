#include "projectile.h"
#include "map.h"
#include <stdlib.h>
#include "monster.h" 
#include "player.h"
#include <math.h>
#include "raymath.h"

#define FIREBALL_ANIM_SPEED 0.10f
#define FIREBALL_FRAMES 7
#define FIREBALL_SCALE 2.5f

static bool fireballLoaded = false;
static Texture2D fireballTex;

static void LoadFireballOnce(void) {
    TraceLog(LOG_INFO, "CWD: %s", GetWorkingDirectory());
    TraceLog(LOG_INFO, "Trying: %s",
             "resources/projectiles/wizard/fireball/Wizard-Fireball.png");
    if (fireballLoaded) return;
    fireballTex = LoadTexture("resources/projectiles/wizard/fireball/Wizard-Fireball.png");
    fireballLoaded = true;
}

static void UnloadFireballOnce(void) {
    if (!fireballLoaded) return;
    if (fireballTex.id) UnloadTexture(fireballTex);
    fireballLoaded = false;
}


// A lista de projéteis
static ProjectileNode *listaDeProjeteis = NULL;

// Função privada para desenhar 1 projétil
static void DrawOneProjectile(Projectile p) {
    if (!fireballLoaded || fireballTex.id <= 0) {
        DrawCircleV(p.position, p.radius, p.color);
        return;
    }

    float frameWidth = (float)fireballTex.width / FIREBALL_FRAMES;

    Rectangle sourceRec = {
        frameWidth * p.currentFrame,
        0.0f,
        frameWidth,
        (float)fireballTex.height
    };

    Rectangle destRec = {
        p.position.x,
        p.position.y,
        frameWidth * FIREBALL_SCALE,
        (float)fireballTex.height * FIREBALL_SCALE
    };

    Vector2 origin = { destRec.width/2.0f, destRec.height/2.0f };

    // rotaciona conforme direção do tiro
    float angle = atan2f(p.direction.y, p.direction.x) * 57.2957795f;

    DrawTexturePro(fireballTex, sourceRec, destRec, origin, angle, WHITE);
}


// Função privada para atualizar 1 projétil
static void UpdateOneProjectile(Projectile *p) {
    p->position.x += p->direction.x * p->speed;
    p->position.y += p->direction.y * p->speed;

    // animação
    p->frameTime += GetFrameTime();
    if (p->frameTime >= FIREBALL_ANIM_SPEED) {
        p->frameTime = 0;
        p->currentFrame = (p->currentFrame + 1) % FIREBALL_FRAMES;
    }
 
}


void SpawnProjectile(Vector2 position, Vector2 direction) {
    LoadFireballOnce();

    ProjectileNode *novo = (ProjectileNode *)malloc(sizeof(ProjectileNode));
    if (novo == NULL) return;

    if (direction.x == 0 && direction.y == 0) direction = (Vector2){0, -1};
    direction = Vector2Normalize(direction);

    novo->data.position = position;
    novo->data.direction = direction;
    novo->data.speed = 5.0f;
    novo->data.radius = 5.0f;
    novo->data.color = YELLOW;

    novo->data.currentFrame = 0;
    novo->data.frameTime = 0.0f;

    novo->next = listaDeProjeteis;
    listaDeProjeteis = novo;
}


void UpdateProjectiles(Map *map, Player *p) {

    ProjectileNode *prev = NULL;
    ProjectileNode *current = listaDeProjeteis;
    
    while (current != NULL) {
        UpdateOneProjectile(&(current->data));
        
        ProjectileNode *nextNode = current->next; 

        // colisão com o mapa
        float r = current->data.radius;
        Vector2 pos = current->data.position;

        bool colidiuComMapa =
            CheckMapCollision(*map, pos) ||
            CheckMapCollision(*map, (Vector2){ pos.x + r, pos.y }) ||
            CheckMapCollision(*map, (Vector2){ pos.x - r, pos.y }) ||
            CheckMapCollision(*map, (Vector2){ pos.x, pos.y + r }) ||
            CheckMapCollision(*map, (Vector2){ pos.x, pos.y - r });

        // Prepara Hitbox do Projétil
        Rectangle projRect = {
            .x = current->data.position.x - current->data.radius,
            .y = current->data.position.y - current->data.radius,
            .width = current->data.radius * 2,
            .height = current->data.radius * 2
        };
        
        
        int pontosGanhos = CheckMonsterCollision(projRect);
        
        if (pontosGanhos > 0) {
             p->score += pontosGanhos; 
        }
        
        if (colidiuComMapa || pontosGanhos > 0) {
            
            if (prev == NULL) {
                listaDeProjeteis = nextNode;
            } else {
                prev->next = nextNode;
            }
            
            free(current); 
            
        } else {
            prev = current;
        }

        current = nextNode;
    }

    if (listaDeProjeteis == NULL) {
        UnloadFireballOnce();
    }


}

void DrawProjectiles(void) {
    ProjectileNode *current = listaDeProjeteis;
    while (current != NULL) {
        DrawOneProjectile(current->data);
        current = current->next;
    }
}

void UnloadProjectiles(void) {
    ProjectileNode *current = listaDeProjeteis;
    ProjectileNode *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    listaDeProjeteis = NULL;
    UnloadFireballOnce();
}