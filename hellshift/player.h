#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"
#include <stdbool.h>
#include "map.h"

typedef enum CharacterClass {
    CLASS_MAGO,
    CLASS_GUERREIRO
} Class;

typedef enum {
    PLAYER_IDLE,
    PLAYER_WALK,
    PLAYER_ATTACK,
    PLAYER_SPECIAL,
    PLAYER_HURT,
    PLAYER_DEATH,     
    PLAYER_REBIRTH,   
    PLAYER_GHOST
} PlayerAnimState;

typedef struct Player{
    Vector2 position;
    float speed;
    int life;
    int maxLife;      
    int score;
    Color color;
    Color originalColor;
    Class playerclass;
    
    KeyboardKey keyUp;
    KeyboardKey keyDown;
    KeyboardKey keyLeft;
    KeyboardKey keyRight;
    KeyboardKey keyAction;
    bool ghost;          
    float damageCooldown;
    bool ready;
    float attackTimer;
    bool isPossessing;          // ta possuindo alguém?
    void *possessedMonster;     // ponteiro para o monstro (void* para evitar ciclo, cast no .c)
    bool trapActive;

    // sprites e animação
    Texture2D texIdle;
    Texture2D texWalk;
    Texture2D texAttack;
    Texture2D texSpecial;
    Texture2D texHurt;
    Texture2D texDeath;

    // controle de Animação
    PlayerAnimState state;   // ação (andando, parado...)
    float frameTime;         // contador de tempo para trocar frame
    int currentFrame;        
    int facingDirection;     // 1 = Direita, (para splitar) -1 = Esquerda
    bool isAttacking;        
} Player;

void InitPlayer(Player *p);
void InitPlayerClassStats(Player *p);
bool shouldDrawPlayer(Player p);
void UpdatePlayer(Player *p, Map *map, Player *other);
void DrawPlayer(Player p);
void UnloadPlayerTextures(Player *p);


#endif 