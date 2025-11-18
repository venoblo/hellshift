#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"
#include <stdbool.h>

typedef struct Map Map;

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
    Class playerclass;
    
    KeyboardKey keyUp;
    KeyboardKey keyDown;
    KeyboardKey keyLeft;
    KeyboardKey keyRight;
    KeyboardKey keyAction;
    bool ghost;          
    float damageCooldown;
    bool ready;

} Player;

void InitPlayer(Player *p);
void UpdatePlayer(Player *p, Map *map);
void DrawPlayer(Player p);


#endif 