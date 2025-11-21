#include "raylib.h"
#include "raymath.h"
#include "player.h"
#include "monster.h"    
#include "projectile.h"
#include "map.h"
#include <stdio.h> 

typedef enum GameScreen { 
    SCREEN_MAIN_MENU,     
    SCREEN_SAVE_SELECT,   
    SCREEN_NUM_PLAYERS,   
    SCREEN_CLASS_SELECT,  
    SCREEN_GAMEPLAY,      
    SCREEN_OPTIONS,
    SCREEN_GAMEOVER,
    SCREEN_CREDITS
} GameScreen;

Map mapa;
int numPlayers = 1; 
int mainMenuSelection = 0; 
int saveSlotSelection = 0; 

#define MAP_OFFSET_X 0 
#define MAP_OFFSET_Y 0 

Player p1 = {
    .position = (Vector2){350, 225}, 
    .speed = 3.0f, .life = 100, .maxLife = 100, .score = 0, .color = BLUE, .originalColor = BLUE,
    .playerclass = CLASS_MAGO, .ghost = false, .damageCooldown = 0, .ready = false,
    .keyUp = KEY_W, .keyDown = KEY_S, .keyLeft = KEY_A, .keyRight = KEY_D, .keyAction = KEY_SPACE
};

Player p2 = {
    .position = (Vector2){450, 225}, 
    .speed = 3.0f, .life = 100, .maxLife = 100, .score = 0, .color = GREEN, .originalColor = GREEN,
    .playerclass = CLASS_GUERREIRO, .ghost = false, .damageCooldown = 0, .ready = false,
    .keyUp = KEY_UP, .keyDown = KEY_DOWN, .keyLeft = KEY_LEFT, .keyRight = KEY_RIGHT, .keyAction = KEY_ENTER
};

bool level1Started = false;

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "Hellshift - Final");
    LoadMap(&mapa, "level1.txt");
    SetTargetFPS(60); 
    SetExitKey(0); 

    GameScreen currentScreen = SCREEN_MAIN_MENU; 

    while (!WindowShouldClose())
    {
        // ============================================================
        // 1. LÓGICA (UPDATE) - Acontece ANTES de desenhar
        // ============================================================
        
        if (currentScreen == SCREEN_MAIN_MENU) {
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) mainMenuSelection--;
            if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) mainMenuSelection++;
            if (mainMenuSelection < 0) mainMenuSelection = 4;
            if (mainMenuSelection > 4) mainMenuSelection = 0;

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                switch (mainMenuSelection) {
                    case 0: currentScreen = SCREEN_SAVE_SELECT; break; 
                    case 4: CloseWindow(); break; 
                    default: currentScreen = SCREEN_SAVE_SELECT; break;
                }
            }
        }

        else if (currentScreen == SCREEN_SAVE_SELECT) {
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) saveSlotSelection--;
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) saveSlotSelection++;
            if (saveSlotSelection < 0) saveSlotSelection = 2;
            if (saveSlotSelection > 2) saveSlotSelection = 0;

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) currentScreen = SCREEN_NUM_PLAYERS;
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MAIN_MENU;
        }

        else if (currentScreen == SCREEN_NUM_PLAYERS) {
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                numPlayers = (numPlayers == 1) ? 2 : 1;
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) currentScreen = SCREEN_CLASS_SELECT;
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_SAVE_SELECT;
        }

        else if (currentScreen == SCREEN_CLASS_SELECT) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_NUM_PLAYERS;
                p1.ready = false; p2.ready = false;
            }
            // P1
            if (!p1.ready) {
                if (IsKeyPressed(p1.keyLeft) || IsKeyPressed(p1.keyRight)) 
                    p1.playerclass = (p1.playerclass == CLASS_MAGO) ? CLASS_GUERREIRO : CLASS_MAGO;
                if (IsKeyPressed(p1.keyAction)) p1.ready = true;
            } else if (IsKeyPressed(p1.keyAction)) p1.ready = false;

            // P2
            if (numPlayers == 2) {
                if (!p2.ready) {
                    if (IsKeyPressed(p2.keyLeft) || IsKeyPressed(p2.keyRight)) 
                        p2.playerclass = (p2.playerclass == CLASS_MAGO) ? CLASS_GUERREIRO : CLASS_MAGO;
                    if (IsKeyPressed(p2.keyAction)) p2.ready = true;
                } else if (IsKeyPressed(p2.keyAction)) p2.ready = false;
            }

            if (p1.ready && (numPlayers == 1 || p2.ready)) {
                InitPlayerClassStats(&p1);
                if (numPlayers == 2) InitPlayerClassStats(&p2);
                
                // Reset Pos
                p1.position = (Vector2){350, 225};
                p2.position = (Vector2){450, 225};
                
                currentScreen = SCREEN_GAMEPLAY;
            }
        }

        else if (currentScreen == SCREEN_GAMEPLAY) {
            // ESC -> VOLTAR
            if (IsKeyPressed(KEY_ESCAPE)) {
                p1.ready = false; p2.ready = false;
                level1Started = false; 
                UnloadMonsters(); 
                currentScreen = SCREEN_CLASS_SELECT;
            }

            // Spawn Inicial
            if (!level1Started) {
                SpawnMonster((Vector2){100, 100}, MONSTER_SKELETON);
                SpawnMonster((Vector2){640, 120}, MONSTER_SKELETON);
                SpawnMonster((Vector2){120, 350}, MONSTER_SKELETON);
                SpawnMonster((Vector2){640, 200}, MONSTER_SKELETON);
                level1Started = true;
            }

            // --- LÓGICA DE VIDA E FANTASMA ---
            
            // 1 PLAYER: Morte = Game Over Direto
            if (numPlayers == 1) {
                if (p1.life <= 0) {
                    currentScreen = SCREEN_GAMEOVER;
                } else {
                    UpdatePlayer(&p1, &mapa);
                }
            }
            // 2 PLAYERS: Morte = Fantasma. 2 Mortes = Game Over
            else {
                if (p1.life <= 0) p1.ghost = true;
                if (p2.life <= 0) p2.ghost = true;

                if (p1.ghost && p2.ghost) {
                    currentScreen = SCREEN_GAMEOVER;
                }

                UpdatePlayer(&p1, &mapa);
                UpdatePlayer(&p2, &mapa);
            }

            // --- DANOS E COLISÕES ---
            if (currentScreen == SCREEN_GAMEPLAY) { // Só processa se ainda estiver no jogo
                if (!p1.ghost && p1.damageCooldown <= 0 && CheckPlayerHit(p1.position, 12.0f)) {
                    p1.life -= 20; p1.damageCooldown = 1.0f;
                }
                if (numPlayers == 2 && !p2.ghost && p2.damageCooldown <= 0 && CheckPlayerHit(p2.position, 12.0f)) {
                    p2.life -= 20; p2.damageCooldown = 1.0f;
                }

                // Bombas
                if (numPlayers == 2) {
                    if (p1.ghost && p1.trapActive && !p2.ghost && Vector2Distance(p1.position, p2.position) < 100.0f) {
                        p2.life -= 80; p2.damageCooldown = 1.0f;
                    }
                    if (p2.ghost && p2.trapActive && !p1.ghost && Vector2Distance(p2.position, p1.position) < 100.0f) {
                        p1.life -= 80; p1.damageCooldown = 1.0f;
                    }
                }

                UpdateMonsters(&p1, &p2, numPlayers, &mapa); 
                UpdateProjectiles(&mapa, &p1);
            }
        }
        
        else if (currentScreen == SCREEN_GAMEOVER) {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
                // RESET TOTAL
                level1Started = false;
                p1.ready = false; p2.ready = false;
                UnloadMonsters(); 
                UnloadProjectiles();
                currentScreen = SCREEN_MAIN_MENU;
            }
        }
        
        else if (currentScreen == SCREEN_OPTIONS || currentScreen == SCREEN_CREDITS) {
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MAIN_MENU;
        }

        // ============================================================
        // 2. DESENHO (DRAW) - Apenas visualização
        // ============================================================
        BeginDrawing();
            ClearBackground((Color){20, 20, 30, 255});

            if (currentScreen == SCREEN_MAIN_MENU) {
                DrawText("HELLSHIFT", 300, 50, 40, RED);
                const char* options[] = {"NOVO JOGO", "CARREGAR JOGO", "OPCOES", "CREDITOS", "SAIR"};
                for (int i = 0; i < 5; i++) {
                    Color c = (i == mainMenuSelection) ? YELLOW : GRAY;
                    DrawText(options[i], 320, 150 + i*40, 20, c);
                    if (i == mainMenuSelection) DrawTriangle((Vector2){300, 160 + i*40}, (Vector2){290, 150 + i*40}, (Vector2){290, 170 + i*40}, RED);
                }
            }
            else if (currentScreen == SCREEN_SAVE_SELECT) {
                DrawText("SELECIONE UM SLOT", 250, 50, 30, WHITE);
                for (int i = 0; i < 3; i++) {
                    int posX = 100 + i * 200; 
                    Rectangle slotRect = {posX, 150, 150, 120};
                    if (i == saveSlotSelection) {
                        DrawRectangleRec(slotRect, RED); DrawRectangleLinesEx(slotRect, 4, YELLOW);
                    } else DrawRectangleRec(slotRect, DARKGRAY);
                    DrawText(TextFormat("SLOT %d", i+1), posX + 40, 200, 20, WHITE);
                }
                DrawText("ENTER para confirmar", 300, 400, 20, GRAY);
            }
            else if (currentScreen == SCREEN_NUM_PLAYERS) {
                int opt1X = 250; int opt2X = 500; int optY = 200;  
                DrawText("QUANTOS JOGADORES?", 250, 100, 30, WHITE);
                DrawText("1 JOGADOR", opt1X, optY, 30, (numPlayers == 1) ? YELLOW : GRAY);
                DrawText("2 JOGADORES", opt2X, optY, 30, (numPlayers == 2) ? YELLOW : GRAY);
                int arrowX = (numPlayers == 1) ? opt1X : opt2X;
                DrawTriangle((Vector2){arrowX - 20, optY + 15}, (Vector2){arrowX - 35, optY}, (Vector2){arrowX - 35, optY + 30}, RED);
            }
            else if (currentScreen == SCREEN_CLASS_SELECT) {
                DrawText("SELECAO DE CLASSE", 280, 50, 30, WHITE);
                DrawText("P1", 150, 150, 30, BLUE);
                DrawText(p1.playerclass == CLASS_MAGO ? "MAGO" : "GUERREIRO", 150, 200, 20, YELLOW);
                if (p1.ready) DrawText("OK", 150, 250, 20, GREEN);
                if (numPlayers == 2) {
                    DrawText("P2", 550, 150, 30, GREEN);
                    DrawText(p2.playerclass == CLASS_MAGO ? "MAGO" : "GUERREIRO", 550, 200, 20, YELLOW);
                    if (p2.ready) DrawText("OK", 550, 250, 20, GREEN);
                }
            }
            else if (currentScreen == SCREEN_GAMEPLAY) {
                DrawMap(mapa); 
                if (p1.life > 0 || p1.ghost) DrawPlayer(p1);
                if (numPlayers == 2 && (p2.life > 0 || p2.ghost)) DrawPlayer(p2);
                DrawMonsters();
                DrawProjectiles();
                DrawText(TextFormat("P1 HP: %d", p1.life), 10, 10, 20, BLUE);
                if (numPlayers == 2) DrawText(TextFormat("P2 HP: %d", p2.life), screenWidth - 100, 10, 20, GREEN);
                DrawText(TextFormat("Score: %d", p1.score + p2.score), 350, 10, 20, YELLOW);
                if (GetMonsterCount() == 0) DrawText("PORTA ABERTA!", 350, 400, 20, ORANGE);
            }
            
            // --- DESENHO GAME OVER (AGORA VAI APARECER) ---
            else if (currentScreen == SCREEN_GAMEOVER) {
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 200}); 
                DrawText("GAME OVER", 280, 150, 50, RED);
                DrawText(TextFormat("SCORE FINAL: %d", p1.score + p2.score), 300, 250, 30, YELLOW);
                DrawText("APERTE ESC PARA VOLTAR", 280, 350, 20, GRAY);
            }
            
            else if (currentScreen == SCREEN_OPTIONS) DrawText("OPCOES", 350, 200, 20, WHITE);
            else if (currentScreen == SCREEN_CREDITS) DrawText("CREDITOS", 350, 200, 20, WHITE);

        EndDrawing();
    }

    UnloadMonsters();
    UnloadProjectiles();
    CloseWindow(); 
    return 0;
}