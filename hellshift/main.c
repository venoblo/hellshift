#include "raylib.h"
#include "player.h"
#include "monster.h"    
#include "projectile.h"
#include "map.h"
#include <stdio.h> 

typedef enum GameScreen { 
    SCREEN_NUM_PLAYERS, 
    SCREEN_CLASS_SELECT, 
    SCREEN_GAMEPLAY,
    SCREEN_GAMEOVER 
} GameScreen;

Map mapa;
int numPlayers = 1; 

// Inicializa os jogadores (mantido como global)
Player p1 = {
    .position = (Vector2){300, 225}, .speed = 3.0f, .life = 5, .maxLife = 5, 
    .score = 0, .color = BLUE, .playerclass = CLASS_MAGO, 
    .ghost = false, .damageCooldown = 0, .ready = false,
    .keyUp = KEY_W, .keyDown = KEY_S, .keyLeft = KEY_A, .keyRight = KEY_D, .keyAction = KEY_SPACE
};

Player p2 = {
    .position = (Vector2){500, 225}, .speed = 3.0f, .life = 5, .maxLife = 5, 
    .score = 0, .color = GREEN, .playerclass = CLASS_MAGO, 
    .ghost = false, .damageCooldown = 0, .ready = false,
    .keyUp = KEY_UP, .keyDown = KEY_DOWN, .keyLeft = KEY_LEFT, .keyRight = KEY_RIGHT, .keyAction = KEY_ENTER
};


int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Hellshift - Rodando!");

    LoadMap(&mapa, "level1.txt"); 

    SetTargetFPS(60); 

    GameScreen currentScreen = SCREEN_NUM_PLAYERS; 

    // Criar monstros 
    SpawnMonster((Vector2){100, 100}, MONSTER_SLIME);
    SpawnMonster((Vector2){200, 200}, MONSTER_ZOMBIE);
    SpawnMonster((Vector2){300, 100}, MONSTER_SKELETON);

    while (!WindowShouldClose())
    {
        // ============================================================
        // LÓGICA DO UPDATE (FLUXO DE ESTADOS)
        // ============================================================
        
        if (currentScreen == SCREEN_NUM_PLAYERS) {
            // Lógica de seleção de 1 ou 2 jogadores (HORIZONTAL)
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                numPlayers = (numPlayers == 1) ? 2 : 1;
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                currentScreen = SCREEN_CLASS_SELECT;
            }
            // NOVO: ESC fecha o jogo se estiver na primeira tela
            if (IsKeyPressed(KEY_ESCAPE)) {
                return 0; // Sai do main e fecha a janela
            }
        }
        
        else if (currentScreen == SCREEN_CLASS_SELECT) {
            // NOVO: Voltar para a seleção de jogadores
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_NUM_PLAYERS;
                // Reseta o estado dos jogadores para a próxima tentativa
                p1.ready = false; 
                p2.ready = false;
            }

            // --- LÓGICA DE ESCOLHA (JOGADOR 1) ---
            if (!p1.ready) {
                if (IsKeyPressed(p1.keyLeft) || IsKeyPressed(p1.keyRight)) {
                    p1.playerclass = (p1.playerclass == CLASS_MAGO) ? CLASS_GUERREIRO : CLASS_MAGO;
                }
                if (IsKeyPressed(p1.keyAction)) p1.ready = true;
            } else {
                 if (IsKeyPressed(p1.keyAction)) p1.ready = false; 
            }

            // --- LÓGICA DE ESCOLHA (JOGADOR 2 - SÓ SE numPlayers == 2) ---
            if (numPlayers == 2) {
                if (!p2.ready) {
                    if (IsKeyPressed(p2.keyLeft) || IsKeyPressed(p2.keyRight)) {
                        p2.playerclass = (p2.playerclass == CLASS_MAGO) ? CLASS_GUERREIRO : CLASS_MAGO;
                    }
                    if (IsKeyPressed(p2.keyAction)) p2.ready = true;
                } else {
                    if (IsKeyPressed(p2.keyAction)) p2.ready = false; 
                }
            }

            // --- TRANSIÇÃO PARA GAMEPLAY ---
            if (p1.ready && (numPlayers == 1 || p2.ready)) {
                InitPlayerClassStats(&p1); 
                if (numPlayers == 2) {
                    InitPlayerClassStats(&p2);
                }
                currentScreen = SCREEN_GAMEPLAY;
            }
        }
        
        else if (currentScreen == SCREEN_GAMEPLAY) {
            // NOVO: Voltar para a seleção de classes
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_CLASS_SELECT;
            }

            // Movimento e Lógica de Jogo
            if (p1.life <= 0) p1.ghost = true;
            UpdatePlayer(&p1, &mapa);

            if (numPlayers == 2) {
                if (p2.life <= 0) p2.ghost = true;
                UpdatePlayer(&p2, &mapa); // <-- CHAMA UPDATE P2
            }

            UpdateMonsters(&p1, &mapa); 
            UpdateProjectiles(&mapa, &p1); 

            if (p1.ghost && (numPlayers == 1 || p2.ghost)) {
        
            }
        }
        
        // ============================================================
        // DESENHO/DRAW
        // ============================================================
        BeginDrawing();
            ClearBackground((Color){30, 30, 40, 255});
            
            if (currentScreen == SCREEN_NUM_PLAYERS) {
                // Desenho do menu principal (com seleção horizontal e setas)
                int opt1X = 250; 
                int opt2X = 500; 
                int optY = 200;  
                int arrowSize = 15;
                
                DrawText("HELLSHIFT - JOGADORES", screenWidth/2 - 250, 50, 40, WHITE);
                
                // MUDANÇA: Indicando controles horizontais
                DrawText("A/D ou SETAS ESQ/DIR para alterar", screenWidth/2 - 250, 150, 20, GRAY);
                
                // --- DESENHO DAS DUAS OPÇÕES ---
                DrawText("1 JOGADOR", opt1X, optY, 30, (numPlayers == 1) ? YELLOW : GRAY);
                DrawText("2 JOGADORES", opt2X, optY, 30, (numPlayers == 2) ? YELLOW : GRAY);
                
                // --- DESENHO DO SELETOR (SETINHA) ---
                int arrowX = (numPlayers == 1) ? opt1X : opt2X;
                DrawTriangle(
                    (Vector2){arrowX - 20, optY + arrowSize / 2},         
                    (Vector2){arrowX - 35, optY - arrowSize + 5},         
                    (Vector2){arrowX - 35, optY + arrowSize + 5},         
                    RED
                );

                DrawText("ENTER/ESPAÇO para continuar", screenWidth/2 - 250, 350, 20, GRAY);
                DrawText("APERTE ESC PARA SAIR DO JOGO", screenWidth/2 - 150, screenHeight - 30, 20, LIGHTGRAY);
            }
            
            else if (currentScreen == SCREEN_CLASS_SELECT) {
                // Desenho do Menu de Classes (P1 e P2)
                DrawText("P1 (A/D p/ classe)", 100, 100, 20, BLUE);
                DrawText(p1.playerclass == CLASS_MAGO ? "MAGO" : "GUERREIRO", 100, 130, 20, YELLOW);
                if (p1.ready) DrawText("PRONTO!", 100, 170, 20, GREEN);
                DrawCircleV(p1.position, 20, p1.color);

                if (numPlayers == 2) {
                    DrawText("P2 (Setas E/D)", 500, 100, 20, GREEN);
                    DrawText(p2.playerclass == CLASS_MAGO ? "MAGO" : "GUERREIRO", 500, 130, 20, YELLOW);
                    if (p2.ready) DrawText("PRONTO!", 500, 170, 20, GREEN);
                    DrawCircleV(p2.position, 20, p2.color);
                } else {
                     DrawText("JOGADOR 1: APERTE ESPAÇO P/ INICIAR", 150, 350, 20, GRAY);
                }
                
                // NOVO: Instrução de voltar
                DrawText("APERTE ESC PARA VOLTAR", screenWidth/2 - 150, screenHeight - 30, 20, LIGHTGRAY);
            }


            else if (currentScreen == SCREEN_GAMEPLAY) {
                // Desenho do Jogo
                DrawRectangle(0, 0, screenWidth, screenHeight, DARKGRAY); 
                
                // Desenho dos elementos do jogo (condicionais)
                if (p1.life > 0 || p1.ghost) DrawPlayer(p1);
                if (numPlayers == 2 && (p2.life > 0 || p2.ghost)) DrawPlayer(p2);
                
                DrawMonsters();
                DrawProjectiles();
                
                // HUD
                DrawText(TextFormat("P1: %d", p1.life), 10, 10, 20, BLUE);
                if (numPlayers == 2) {
                    DrawText(TextFormat("P2: %d", p2.life), screenWidth - 120, 10, 20, GREEN);
                }
                
                // NOVO: Instrução de voltar
                DrawText("APERTE ESC PARA VOLTAR AO MENU DE CLASSES", screenWidth/2 - 250, screenHeight - 30, 20, LIGHTGRAY);
            }

        EndDrawing();
    }

    // 3. Finalização
    UnloadMonsters();
    UnloadProjectiles();
    CloseWindow(); 

    return 0;
}