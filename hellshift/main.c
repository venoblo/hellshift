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
        .maxLife = 5,       
        .score = 0,         
        .damageCooldown = 0,
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
        UpdatePlayer(&player, &mapa);
        UpdateMonsters(&player);

        if (player.damageCooldown <= 0) {
            // O raio do jogador é 12 (do DrawPlayer)
            if (CheckPlayerHit(player.position, 12.0f)) {
                player.life--;
                player.damageCooldown = 1.0f; // 1 segundo de invencibilidade
            }
        }

        if (player.life <= 0) {
            // (Futuramente, aqui chamaremos a tela de Game Over e Salvar Score)
            break; // Por enquanto, só quebra o loop
        }

        UpdateMonsters(&player, &mapa);
        UpdateProjectiles(&mapa, &player);


        // --- DESENHO/DRAW (O que aparece na tela) ---
        BeginDrawing();

            ClearBackground((Color) (30, 30, 40, 255)); // Limpa a tela com a cor branca
            DrawMap(mapa);
            
            DrawText("Hellshift - teste", 190, 200, 20, LIGHTGRAY);

            DrawPlayer(player);
            DrawMonsters();

            DrawText(TextFormat("SCORE: %05d", player.score), 10, 10, 20, YELLOW);
            DrawText(TextFormat("LIFE: %d / %d", player.life, player.maxLife), 10, 35, 20, RED);

        EndDrawing();
    }

    // 3. Finalização
    UnloadMonsters();
    UnloadProjectiles();
    CloseWindow(); // Fecha a janela e libera os recursos

    return 0;
}