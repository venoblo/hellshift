#include "save.h"
#include <stdio.h>
#include <time.h> 

//dá nome aos arquivos de saves
void GetSaveFileName(int slot, char *buffer) {
    sprintf(buffer, "save_slot_%d.dat", slot);
}

void SaveGame(int slot, SaveData data) {
    char filename[30];
    GetSaveFileName(slot, filename);

    // pega a hora do sistema 
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(data.dateBuffer, "%02d/%02d/%d %02d:%02d", 
            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, 
            tm.tm_hour, tm.tm_min);

    data.isActive = true; 

    FILE *file = fopen(filename, "wb");
    if (file != NULL) {
        fwrite(&data, sizeof(SaveData), 1, file);
        fclose(file);
    }
}

SaveData LoadGameData(int slot) {
    char filename[30];
    GetSaveFileName(slot, filename);

    SaveData data = {0}; // Zera tudo
    data.isActive = false;

    FILE *file = fopen(filename, "rb");
    if (file != NULL) {
        fread(&data, sizeof(SaveData), 1, file);
        fclose(file);
    }
    return data;
}

bool SaveExists(int slot) {
    SaveData data = LoadGameData(slot);
    return data.isActive;
}