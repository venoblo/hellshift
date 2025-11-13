#include "raylib.h"
#include "player.h"
#include "monster.h"    
#include "projectile.h"
#include "map.h"

Map mapa;

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Hellshift - Teste");

    LoadMap(&mapa, "arena.txt");

    SetTargetFPS(60); // Define o FPS (Frames Per Second)

    // criar jogador
    Player player = {
        .position = (Vector2){screenWidth/2, screenHeight/2},
        .speed = 3.0f,
        .life = 5,
        .color = BLUE,
        .playerclass = CLASS_MAGO
    };


    //Criar monstro
    SpawnMonster((Vector2){100, 100}, MONSTER_SLIME);
    SpawnMonster((Vector2){200, 200}, MONSTER_ZOMBIE);
    SpawnMonster((Vector2){300, 100}, MONSTER_SKELETON);

    // 2. Loop Principal do Jogo
    while (!WindowShouldClose())    // Executa enquanto o usuário não fechar a janela
    {
        // --- (Lógica do Jogo) ---
        UpdatePlayer(&player);
        UpdateMonsters(&player);

        // --- DESENHO/DRAW (O que aparece na tela) ---
        BeginDrawing();

            ClearBackground((Color) {30, 30, 40, 255}); // Limpa a tela com a cor branca
            DrawMap(mapa);
            
            DrawText("Hellshift - teste", 190, 200, 20, LIGHTGRAY);

            DrawPlayer(player);
            DrawMonsters();

        EndDrawing();
    }

    // 3. Finalização
    UnloadMonsters();
    CloseWindow(); // Fecha a janela e libera os recursos

    return 0;
}