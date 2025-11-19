#include "raylib.h"
#include "player.h"
#include "monster.h"    
#include "projectile.h"
#include "map.h"

typedef enum GameScreen { 
    SCREEN_MENU, 
    SCREEN_GAMEPLAY,
    SCREEN_GAMEOVER 
} GameScreen;

Map mapa;

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Hellshift - Teste");

    LoadMap(&mapa, "arena.txt");

    SetTargetFPS(60); // Define o FPS (Frames Per Second)

    GameScreen currentScreen = SCREEN_MENU;

    // criar jogador 1
    Player p1 = {
        .position = (Vector2){300, 225},
        .speed = 3.0f, 
        .life = 5, 
        .maxLife = 5, 
        .score = 0, 
        .color = BLUE, 
        .ghost = false, 
        .damageCooldown = 0,
        // Controles P1
        .keyUp = KEY_W, .keyDown = KEY_S, .keyLeft = KEY_A, .keyRight = KEY_D, 
        .keyAction = KEY_SPACE
    };

    Player p2 = {
        .position = (Vector2){500, 225},
        .speed = 3.0f,
        .life = 5, 
        .maxLife = 5, 
        .score = 0, 
        .color = GREEN, 
        .ghost = false, 
        .damageCooldown = 0,
        // Controles P2
        .keyUp = KEY_UP, .keyDown = KEY_DOWN, .keyLeft = KEY_LEFT, .keyRight = KEY_RIGHT, 
        .keyAction = KEY_ENTER
    };




    //Criar monstro
    SpawnMonster((Vector2){100, 100}, MONSTER_SLIME);
    SpawnMonster((Vector2){200, 200}, MONSTER_ZOMBIE);
    SpawnMonster((Vector2){300, 100}, MONSTER_SKELETON);

    // 2. Loop Principal do Jogo
    while (!WindowShouldClose())    // Executa enquanto o usuário não fechar a janela
    {
        
        if (currentScreen == SCREEN_MENU) {
            
            if (!p1.ready) {
                if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D)) {

                    if (p1.playerclass == CLASS_MAGO) p1.playerclass = CLASS_GUERREIRO;
                    else p1.playerclass = CLASS_MAGO;
                }
                if (IsKeyPressed(KEY_SPACE)) p1.ready = true;
            } else {
                 if (IsKeyPressed(KEY_SPACE)) p1.ready = false;
            }

            // --- PLAYER 2 ESCOLHE ---
            if (!p2.ready) {
                if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                    // Alterna entre Mago e Guerreiro
                    if (p2.playerclass == CLASS_MAGO) p2.playerclass = CLASS_GUERREIRO;
                    else p2.playerclass = CLASS_MAGO;
                }
                if (IsKeyPressed(KEY_ENTER)) p2.ready = true;
            } else {
                if (IsKeyPressed(KEY_ENTER)) p2.ready = false; // Cancelar pronto
            }

            // SE AMBOS ESTIVEREM PRONTOS -> INICIAR JOGO
            if (p1.ready && p2.ready) {
                currentScreen = SCREEN_GAMEPLAY;
            }
        }
        
        else if (currentScreen == SCREEN_GAMEPLAY) {
            if (p1.life <= 0) p1.ghost = true;
            UpdatePlayer(&p1, &mapa);
            
            if (p2.life <= 0) p2.ghost = true;
            UpdatePlayer(&p2, &mapa);

            UpdateMonsters(&p1, &mapa);
            UpdateProjectiles(&mapa, &p1); // Pontos vão pro P1 por enquanto
            
            if (p1.ghost && p2.ghost) {
                // Game Over se os dois morrerem
                // currentScreen = SCREEN_GAMEOVER;
            }
        }
        





        // --- DESENHO/DRAW (O que aparece na tela) ---
        BeginDrawing();

            ClearBackground((Color){30, 30, 40, 255}); // Limpa a tela com a cor branca
            DrawMap(mapa);
            
            DrawText("Hellshift - teste", 190, 200, 20, LIGHTGRAY);

            DrawPlayer(p1);
            DrawPlayer(p2);
            DrawMonsters();
            DrawProjectiles();

        EndDrawing();
    }

    // 3. Finalização
    UnloadMonsters();
    UnloadProjectiles();
    UnloadProjectiles();
    CloseWindow(); // Fecha a janela e libera os recursos

    return 0;
}