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
    Color color;
    Class playerclass; // <-- NOVO CAMPO
} Player;
void InitPlayer(Player *p);
void UpdatePlayer(Player *p);
void DrawPlayer(Player p);

#endif 