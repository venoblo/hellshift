#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"
#include <stdbool.h>
#include "map.h"

typedef enum CharacterClass {
    CLASS_MAGO,
    CLASS_GUERREIRO
} Class;

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

} Player;


void InitPlayer(Player *p);
void InitPlayerClassStats(Player *p);
void UpdatePlayer(Player *p, Map *map);
void DrawPlayer(Player p);


#endif 