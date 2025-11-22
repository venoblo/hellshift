#include "raylib.h"
#include "raymath.h"
#include "player.h"
#include "monster.h"    
#include "projectile.h"
#include "map.h"
#include <stdio.h> 
#include "save.h"
#include "string.h"

typedef enum GameScreen { 
    SCREEN_MAIN_MENU,
    SCREEN_SLOT_SELECT,
    SCREEN_NAME_INPUT,     
    SCREEN_CONFIRM_OVERWRITE, 
    SCREEN_MSG_EMPTY,     
    SCREEN_SAVE_SELECT,   
    SCREEN_NUM_PLAYERS,   
    SCREEN_CLASS_SELECT,  
    SCREEN_GAMEPLAY,     
    SCREEN_PAUSE, 
    SCREEN_OPTIONS,
    SCREEN_GAMEOVER,
    SCREEN_CREDITS
} GameScreen;

Map mapa;
int numPlayers = 1; 
int mainMenuSelection = 0; 
int saveSlotSelection = 0; 
int pausemenu = 0;

bool level1Started = false;
bool isSavingMode = false; // true = Salvar/Novo, false = Carregar
bool isPauseMenu = false;

char tempName[21] = "PLAYER"; 
int letterCount = 6;
int framesCounter = 0;

#define MAP_OFFSET_X 0 
#define MAP_OFFSET_Y 0 

// Players Globais para facilitar Save/Load
Player p1, p2;

// Função Auxiliar para resetar jogadores
void ResetPlayers() {
    p1 = (Player){
        .position = (Vector2){350, 225}, .speed = 3.0f, .life = 100, .maxLife = 100, 
        .score = 0, .color = BLUE, .originalColor = BLUE, .playerclass = CLASS_MAGO, 
        .ghost = false, .damageCooldown = 0, .ready = false,
        .keyUp = KEY_W, .keyDown = KEY_S, .keyLeft = KEY_A, .keyRight = KEY_D, .keyAction = KEY_SPACE
    };
    p2 = (Player){
        .position = (Vector2){450, 225}, .speed = 3.0f, .life = 100, .maxLife = 100, 
        .score = 0, .color = GREEN, .originalColor = GREEN, .playerclass = CLASS_GUERREIRO, 
        .ghost = false, .damageCooldown = 0, .ready = false,
        .keyUp = KEY_UP, .keyDown = KEY_DOWN, .keyLeft = KEY_LEFT, .keyRight = KEY_RIGHT, .keyAction = KEY_ENTER
    };

    level1Started = false;
    UnloadMonsters();
    UnloadProjectiles();
}

void PerformSave(int slot) {
    SaveData data = {0};
    strcpy(data.saveName, tempName);
    
    data.score = p1.score + p2.score;
    data.numPlayers = numPlayers;
    data.p1Class = (int)p1.playerclass;
    data.p2Class = (int)p2.playerclass;
    data.p1Life = p1.life;
    data.p2Life = p2.life;
    data.level = 1; 
    
    // SALVA SE O LEVEL JÁ COMEÇOU (Para não spawnar monstros default em cima dos salvos)
    data.level1Started = level1Started;

    // --- SALVA OS MONSTROS ---
    ExportMonsters(&data);
    
    SaveGame(slot, data);
}

// Função para Aplicar o Save carregado no Jogo
void ApplyLoadedGame(SaveData data) {
    numPlayers = data.numPlayers;
    ResetPlayers(); 
    
    p1.score = data.score; 
    p1.playerclass = (Class)data.p1Class;
    p1.life = data.p1Life; // Vida recuperada
    
    if (numPlayers == 2) {
        p2.playerclass = (Class)data.p2Class;
        p2.life = data.p2Life; // Vida recuperada
    }
    
    InitPlayerClassStats(&p1); 
    if (numPlayers == 2) InitPlayerClassStats(&p2);
    
    // IMPORTANTE: Sobrescreve a vida DEPOIS do Init (pois Init reseta para MaxLife)
    p1.life = data.p1Life;
    if (numPlayers == 2) p2.life = data.p2Life;

    // --- CARREGA OS MONSTROS ---
    ImportMonsters(data);
    
    // Recupera o estado do nível (para o main não spawnar monstros novos)
    level1Started = data.level1Started; 

    p1.position = (Vector2){350, 225}; // Ou você poderia salvar a posição exata na struct SaveData!
    p2.position = (Vector2){450, 225};
}

int main(void)
{
    Camera2D camera = {0};
    camera.zoom = 1.0f;
    camera.rotation = 0.0f;
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
                    case 0: // NOVO JOGO
                        isSavingMode = true; 
                        isPauseMenu = false; 
                        currentScreen = SCREEN_SLOT_SELECT; 
                        break; 
                    case 1: // CARREGAR JOGO
                        isSavingMode = false; 
                        isPauseMenu = false;
                        currentScreen = SCREEN_SLOT_SELECT; 
                        break; 
                    case 2: currentScreen = SCREEN_OPTIONS; break;
                    case 3: currentScreen = SCREEN_CREDITS; break;
                    case 4: CloseWindow(); break; 
                }
            }
        }

        // 2. SELEÇÃO DE SLOT (Genérica)
        else if (currentScreen == SCREEN_SLOT_SELECT) {
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) saveSlotSelection--;
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) saveSlotSelection++;
            if (saveSlotSelection < 0) saveSlotSelection = 2;
            if (saveSlotSelection > 2) saveSlotSelection = 0;

            // Voltar
            if (IsKeyPressed(KEY_ESCAPE)) {
                if (isPauseMenu) currentScreen = SCREEN_PAUSE;
                else currentScreen = SCREEN_MAIN_MENU;
            }

            // Confirmar Slot
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                bool exists = SaveExists(saveSlotSelection);

                if (isSavingMode) { 
                    // --- MODO SALVAR ---
                    if (exists) {
                        currentScreen = SCREEN_CONFIRM_OVERWRITE;
                    } else {
                        // Se veio do Pause e slot vazio -> Salva Direto (Mantém nome atual ou padrão)
                        if (isPauseMenu) {
                             PerformSave(saveSlotSelection);
                             currentScreen = SCREEN_GAMEPLAY;
                        } else {
                            // Novo Jogo -> Vai digitar nome
                            letterCount = 0;
                            tempName[0] = '\0'; // Limpa nome
                            currentScreen = SCREEN_NAME_INPUT; 
                        }
                    }
                } else {
                    // --- MODO CARREGAR ---
                    if (exists) {
                        if (isPauseMenu) {
                            // Se estiver no pause, pergunta se quer carregar (perder progresso atual)
                            currentScreen = SCREEN_CONFIRM_OVERWRITE; // Reutilizando tela de confirm
                        } else {
                            SaveData data = LoadGameData(saveSlotSelection);
                            ApplyLoadedGame(data);
                            currentScreen = SCREEN_GAMEPLAY;
                        }
                    } else {
                        currentScreen = SCREEN_MSG_EMPTY;
                    }
                }
            }
        }


        else if (currentScreen == SCREEN_NAME_INPUT) {
            // Captura teclas
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && (letterCount < 20)) {
                    tempName[letterCount] = (char)key;
                    tempName[letterCount+1] = '\0';
                    letterCount++;
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                letterCount--;
                if (letterCount < 0) letterCount = 0;
                tempName[letterCount] = '\0';
            }

            // Confirmar Nome
            if (IsKeyPressed(KEY_ENTER)) {
                // Inicia fluxo de criação de personagem
                ResetPlayers();
                // Salva o arquivo inicial com o nome escolhido
                PerformSave(saveSlotSelection); 
                currentScreen = SCREEN_NUM_PLAYERS;
            }
        }

        
        // --- CONFIRMA SOBRESCRITA ---
        else if (currentScreen == SCREEN_CONFIRM_OVERWRITE) {
            if (IsKeyPressed(KEY_S)) { // SIM
                if (isSavingMode) {
                    // Sobrescrever Save
                    if (isPauseMenu) {
                        // Pause: Salva estado atual
                        PerformSave(saveSlotSelection); 
                        currentScreen = SCREEN_GAMEPLAY;
                    } else {
                        // Novo Jogo: Vai para Input de Nome
                        letterCount = 0; tempName[0] = '\0';
                        currentScreen = SCREEN_NAME_INPUT;
                    }
                } else {
                    // Carregar (Vindo do Pause)
                    SaveData data = LoadGameData(saveSlotSelection);
                    ApplyLoadedGame(data);
                    currentScreen = SCREEN_GAMEPLAY;
                }
            }
            if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_SLOT_SELECT;
            }
        }

        // --- MENSAGEM SLOT VAZIO ---
        else if (currentScreen == SCREEN_MSG_EMPTY) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_SLOT_SELECT;
            }
        }

        // --- MENU DE PAUSA ---
        else if (currentScreen == SCREEN_PAUSE) {
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) pausemenu--;
            if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) pausemenu++;
            if (pausemenu < 0) pausemenu = 3;
            if (pausemenu > 3) pausemenu = 0;

            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_GAMEPLAY; 

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                switch (pausemenu) {
                    case 0: currentScreen = SCREEN_GAMEPLAY; break; 
                    case 1: // SALVAR
                        isSavingMode = true;
                        isPauseMenu = true; // IMPORTANTE: Flag de Pause
                        currentScreen = SCREEN_SLOT_SELECT;
                        break;
                    case 2: // CARREGAR
                        isSavingMode = false;
                        isPauseMenu = true;
                        currentScreen = SCREEN_SLOT_SELECT;
                        break;
                    case 3: // MENU PRINCIPAL
                        currentScreen = SCREEN_MAIN_MENU;
                        break;
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

                PerformSave(saveSlotSelection);
                
                currentScreen = SCREEN_GAMEPLAY;
            }
        }

        else if (currentScreen == SCREEN_GAMEPLAY) {
            // ESC -> VOLTAR
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_PAUSE; // <-- AGORA VAI PARA PAUSE
            }

            // Spawn Inicial
            //if (!level1Started) {
            //    SpawnMonster((Vector2){100, 100}, MONSTER_SKELETON);
            //    SpawnMonster((Vector2){640, 120}, MONSTER_SKELETON);
            //    SpawnMonster((Vector2){120, 350}, MONSTER_SKELETON);
            //    SpawnMonster((Vector2){640, 200}, MONSTER_SKELETON);
            //    level1Started = true;
            //}

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

            Vector2 target = p1.position;

            // Se houver 2 jogadores, centraliza entre eles
            if (numPlayers == 2)
                target = Vector2Lerp(p1.position, p2.position, 0.5f);

            camera.target = target;

            // Calcula distância entre players
            float dist = Vector2Distance(p1.position, p2.position);

            // Zoom dinâmico
            if (numPlayers == 1) {
                camera.zoom = 1.0f;
            }
            else {
                float minZoom = 0.6f;
                float maxZoom = 1.1f;

                float newZoom = 1.2f - (dist / 700.0f);

                if (newZoom < minZoom) newZoom = minZoom;
                if (newZoom > maxZoom) newZoom = maxZoom;

                camera.zoom = newZoom;
            }

            camera.offset = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };

            CheckRoomTransition(&mapa, &p1, &p2, numPlayers);

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

                // ================================
                // IR PRO PRÓXIMO ANDAR (BOSS ROOM)
                // ================================
                Room *r = &mapa.dungeon.rooms[mapa.dungeon.currentRoom];

                if (r->type == ROOM_BOSS && r->portalActive && IsKeyPressed(KEY_E)) {

                    if (mapa.dungeon.floorLevel >= 7) {
                        currentScreen = SCREEN_CREDITS;
                    }
                    else {
                        GoToNextFloor(&mapa, &p1.position, &p2.position, numPlayers);
                    }
                }
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
            
            else if (currentScreen == SCREEN_SLOT_SELECT) {
                DrawText(isSavingMode ? "SALVAR JOGO" : "CARREGAR JOGO", 250, 50, 30, WHITE);
                for (int i = 0; i < 3; i++) {
                    int posX = 100 + i * 200; 
                    Rectangle slotRect = {posX, 150, 150, 120};
                    
                    bool exists = SaveExists(i);
                    SaveData data = {0};
                    if (exists) data = LoadGameData(i);

                    if (i == saveSlotSelection) {
                        DrawRectangleRec(slotRect, RED); 
                    } else DrawRectangleRec(slotRect, DARKGRAY);
                    
                    DrawText(TextFormat("SLOT %d", i+1), posX + 40, 160, 20, WHITE);
                    if (exists) {
                        DrawText(data.saveName, posX + 10, 190, 10, WHITE); // Nome do Save
                        DrawText(data.dateBuffer, posX + 10, 210, 10, LIGHTGRAY);
                        DrawText(TextFormat("Pts: %d", data.score), posX + 10, 230, 10, YELLOW);
                    } else DrawText("VAZIO", posX + 50, 210, 20, GRAY);
                }
            }

            else if (currentScreen == SCREEN_NAME_INPUT) {
                DrawText("DIGITE O NOME DO SAVE", 220, 100, 30, WHITE);
                
                // Caixa de Texto
                DrawRectangle(250, 200, 300, 50, DARKGRAY);
                DrawRectangleLines(250, 200, 300, 50, WHITE);
                
                DrawText(tempName, 260, 210, 30, YELLOW);
                
                // Cursor Piscante
                framesCounter++;
                if ((framesCounter/30)%2 == 0) {
                    DrawText("_", 260 + MeasureText(tempName, 30), 210, 30, YELLOW);
                }
                
                DrawText("ENTER para confirmar", 300, 300, 20, GRAY);
            }

            else if (currentScreen == SCREEN_CONFIRM_OVERWRITE) {
                DrawText("ATENCAO!", 320, 100, 40, RED);
                if (isSavingMode) DrawText("Deseja SOBRESCREVER este save?", 220, 200, 20, WHITE);
                else DrawText("Deseja CARREGAR este save?", 250, 200, 20, WHITE);
                
                DrawText("[S] SIM    [N] NAO", 300, 300, 20, YELLOW);
            }

            else if (currentScreen == SCREEN_MSG_EMPTY) {
                DrawText("SLOT VAZIO!", 300, 200, 30, RED);
                DrawText("Pressione ENTER", 320, 250, 20, GRAY);
            }
            
            else if (currentScreen == SCREEN_PAUSE) {
                DrawRectangle(0,0,screenWidth, screenHeight, (Color){0,0,0,200});
                DrawText("PAUSA", 350, 50, 40, WHITE);
                const char* pOptions[] = {"VOLTAR", "SALVAR JOGO", "CARREGAR JOGO", "SAIR P/ MENU"};
                for (int i = 0; i < 4; i++) {
                    Color c = (i == pausemenu) ? YELLOW : GRAY;
                    DrawText(pOptions[i], 300, 150 + i*50, 20, c);
                    if (i == pausemenu) DrawText(">", 280, 150 + i*50, 20, RED);
                }
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
            else if (currentScreen == SCREEN_GAMEPLAY) 
            {
                // ---------- ÁREA DO MUNDO (com câmera) ----------
                BeginMode2D(camera);

                    DrawMap(mapa);

                    if (p1.life > 0 || p1.ghost) DrawPlayer(p1);
                    if (numPlayers == 2 && (p2.life > 0 || p2.ghost)) DrawPlayer(p2);

                    DrawMonsters();
                    DrawProjectiles();

                EndMode2D();

                // ---------- HUD (fora da câmera) ----------
                bool isTab = IsKeyDown(KEY_TAB);
                DrawMiniMap(&mapa, isTab);

                DrawText(TextFormat("P1 HP: %d", p1.life), 10, 10, 20, BLUE);

                if (numPlayers == 2)
                    DrawText(TextFormat("P2 HP: %d", p2.life), screenWidth - 100, 10, 20, GREEN);

                DrawText(TextFormat("Score: %d", p1.score + p2.score), 350, 10, 20, YELLOW);

                DrawText(TextFormat("FLOOR: %d / 7", mapa.dungeon.floorLevel), 340, 35, 20, RED);

                if (GetMonsterCount() == 0) {
                    Room *r = &mapa.dungeon.rooms[mapa.dungeon.currentRoom];

                    if (r->type == ROOM_BOSS) {
                        r->portalActive = true;
                        DrawText("PRESSIONE [E] PARA DESCER", 280, 400, 20, ORANGE);
                    }
                }
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