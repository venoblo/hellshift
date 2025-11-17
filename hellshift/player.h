#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"

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
    float cooldown;
} Player;

void InitPlayer(Player *p);
void UpdatePlayer(Player *p);
void DrawPlayer(Player p);

#endif 