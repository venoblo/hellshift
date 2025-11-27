#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>
#include "player.h" 
#include "raylib.h"
#define MAX_SAVED_MONSTERS 50

typedef struct MonsterSaveData {
    Vector2 position;
    int type;
    int life;
    Color color;
    float speed;
    float activeRange;
    int skelVariant;
    int orcVariant;

} MonsterSaveData;

typedef struct SaveData {
    bool isActive;
    char saveName[21];
    char dateBuffer[20];
    
    int score;
    int level;
    
    int numPlayers;
    int p1Class;
    int p2Class;
    int p1Life;
    int p2Life;

    // --- DADOS DOS MONSTROS ---
    int monsterCount; // numero de monstros
    MonsterSaveData monsters[MAX_SAVED_MONSTERS]; // O array de dados
    bool level1Started;
} SaveData;

void SaveGame(int slot, SaveData data);
SaveData LoadGameData(int slot);
bool SaveExists(int slot);

#endif