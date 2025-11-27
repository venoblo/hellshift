#include "raylib.h"
#include "raymath.h"
#include "player.h"
#include "monster.h"    
#include "projectile.h"
#include "map.h"
#include <stdio.h> 
#include "save.h"
#include "string.h"

Texture2D texMenuBg;
Texture2D texMenuLogo;
Texture2D texStoryBg;
Music soundtrack;
Font fontMenu;



typedef enum GameScreen { 
    SCREEN_MAIN_MENU,
    SCREEN_SLOT_SELECT,
    SCREEN_NAME_INPUT,     
    SCREEN_CONFIRM_OVERWRITE, 
    SCREEN_MSG_EMPTY,     
    SCREEN_SAVE_SELECT,
    SCREEN_STORY,
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

// cutscene
const char *storyLines[] = {
    "Ah... mais mariposas atraidas pela\nminha chama.",
    "Eu vi voces cruzarem os portoes.\nO Cavaleiro, com sua armadura\nbrilhante escondendo um coracao \nganancioso por gloria.",
    "E o Mago, cuja sede de poder o cegou\npara a prudencia.",
    "Voces vieram buscar o \nCoracao do Submundo, nao foi?\nVieram me destronar. Tolos." ,
    "Voces nao entraram em uma masmorra...\nvoces entraram no meu estomago.",
    "Este lugar nao segue as leis do\nseu mundo.", 
    "Aqui, a vida e uma moeda preciosa,\ne a morte... ah, a morte e apenas\numa mudanca de perspectiva.",
    "Saibam disto: o ar que voces respiram\naqui e feito de traicao.", 
    "Se um de voces cair,\na pedra fria deste lugar\nsugara sua lealdade.",
    "Aquele que morrer primeiro descobrira\nque a inveja dos vivos e uma fome\nque nunca passa.",
    "Voces entrarao como aliados,\nmas imploro que descubram o que se\ntornarao quando o primeiro\nsangue for derramado.",
    "Parta nessa aventura sozinho,\nou acompanhado, porem, saiba que\nha consequencias…"
};
const int STORY_LINES_COUNT = sizeof(storyLines)/sizeof(storyLines[0]);
int storyCurrentLine = 0;
int storyCharIndex   = 0;
float storyTimer     = 0.0f;
const float STORY_CHAR_DELAY = 0.03f; 
bool storyLineFinished = false;


char tempName[21] = "PLAYER"; 
int letterCount = 6;
int framesCounter = 0;

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

    InitWindow(screenWidth, screenHeight, "Hellshift");
    LoadMapTextures();
    LoadMap(&mapa, "level1.txt");
    SetTargetFPS(60); 
    SetExitKey(0); 
    InitAudioDevice();  

    soundtrack = LoadMusicStream("resources/soundtrack/hellshift.mp3");

    soundtrack.looping = true;   //loop
    SetMusicVolume(soundtrack, 0.6f); 

    PlayMusicStream(soundtrack);      

    texMenuBg   = LoadTexture("resources/menu/masmorra-fundo.png");
    texMenuLogo = LoadTexture("resources/menu/HELLSHIFTLOGO.png");
    texStoryBg  = LoadTexture("resources/menu/fundo.png");
    fontMenu = LoadFontEx("resources/fonts/Righteous.ttf", 32, NULL, 0);


    GameScreen currentScreen = SCREEN_MAIN_MENU; 

    while (!WindowShouldClose())
    {
        UpdateMusicStream(soundtrack);

        
        if (currentScreen == SCREEN_MAIN_MENU) {
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) mainMenuSelection--;
            if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) mainMenuSelection++;
            if (mainMenuSelection < 0) mainMenuSelection = 2;
            if (mainMenuSelection > 2) mainMenuSelection = 0;

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

                case 2: // SAIR
                    CloseWindow();
                    break;
            }
            }
        }

        // seleção de slot
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
                currentScreen = SCREEN_STORY;
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

        else if (currentScreen == SCREEN_STORY) {
            float dt = GetFrameTime();

            // Avança as letras aos poucos
            if (!storyLineFinished) {
                storyTimer += dt;
                while (storyTimer >= STORY_CHAR_DELAY && !storyLineFinished) {
                    storyTimer   -= STORY_CHAR_DELAY;
                    storyCharIndex++;

                    int len = (int)strlen(storyLines[storyCurrentLine]);
                    if (storyCharIndex >= len) {
                        storyCharIndex    = len;
                        storyLineFinished = true;
                    }
                }
            }

            // Enter / Espaço
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                int len = (int)strlen(storyLines[storyCurrentLine]);

                if (!storyLineFinished) {
                    // mostra a linha inteira de uma vez
                    storyCharIndex    = len;
                    storyLineFinished = true;
                } else {
                    // Vai para a próxima linha
                    storyCurrentLine++;
                    if (storyCurrentLine >= STORY_LINES_COUNT) {
                        isSavingMode  = true;
                        isPauseMenu   = false;
                        currentScreen = SCREEN_NUM_PLAYERS;

                        // reseta para próxima vez que entrar na história
                        storyCurrentLine   = 0;
                        storyCharIndex     = 0;
                        storyTimer         = 0.0f;
                        storyLineFinished  = false;
                    } else {
                        storyCharIndex     = 0;
                        storyTimer         = 0.0f;
                        storyLineFinished  = false;
                    }
                }
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
            // PLAYER 1
            if (p1.life <= 0 && !p1.ghost && !p1.isPossessing) {
                // Se ainda não começou a morrer, começa agora
                if (p1.state != PLAYER_DEATH && p1.state != PLAYER_REBIRTH && p1.state != PLAYER_GHOST) {
                    p1.state = PLAYER_DEATH;
                    p1.currentFrame = 0;
                    p1.frameTime = 0.0f;
                }
                // Se por algum motivo bugou e ficou IDLE com vida 0, força DEATH de novo
                else if (p1.state == PLAYER_IDLE || p1.state == PLAYER_WALK) {
                     p1.state = PLAYER_DEATH;
                     p1.currentFrame = 0;
                }
            }

            // PLAYER 2 (Se existir)
            if (numPlayers == 2 && p2.life <= 0 && !p2.ghost && !p2.isPossessing) {
                if (p2.state != PLAYER_DEATH && p2.state != PLAYER_REBIRTH && p2.state != PLAYER_GHOST) {
                    p2.state = PLAYER_DEATH;
                    p2.currentFrame = 0;
                    p2.frameTime = 0.0f;
                }
                else if (p2.state == PLAYER_IDLE || p2.state == PLAYER_WALK) {
                     p2.state = PLAYER_DEATH;
                     p2.currentFrame = 0;
                }
            }


            // --- 2. CHECAGEM DE GAME OVER ---
            bool gameOver = false;

            bool p1Out = (p1.ghost || p1.isPossessing);  // isPossessing só acontece quando já era fantasma
            bool p2Out = (p2.ghost || p2.isPossessing);

            if (numPlayers == 1) {
                if (p1Out) gameOver = true;
            } else {
                if (p1Out && p2Out) gameOver = true;
            }

            if (gameOver) {
                currentScreen = SCREEN_GAMEOVER;
            }

            UpdatePlayer(&p1, &mapa, (numPlayers == 2 ? &p2 : NULL));
            if (numPlayers == 2) UpdatePlayer(&p2, &mapa, &p1);

            
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
                

                // -------- DANO MANUAL DO POSSUÍDO (sem IA) --------
                if (numPlayers == 2) {

                    // P1 possuindo ataca P2
                    if (p1.isPossessing) {
                        MonsterNode *m = (MonsterNode*)p1.possessedMonster;
                        if (m != NULL && m->data.state == MONSTER_ATTACK && p2.life > 0 && !p2.ghost && p2.damageCooldown <= 0) {

                            Vector2 monsterCenter = { m->data.position.x + 15, m->data.position.y + 15 };
                            float dist = Vector2Distance(monsterCenter, p2.position);

                            // mesmo critério do CheckPlayerHit: playerRadius(40) + monsterRadius(15)
                            if (dist < (40.0f + 15.0f)) {
                                p2.life -= m->data.damage;
                                p2.damageCooldown = 0.6f; 

                                if (p2.playerclass == CLASS_GUERREIRO) {
                                    p2.state = PLAYER_HURT;
                                    p2.currentFrame = 0;
                                    p2.frameTime = 0.0f;
                                }
                            }
                        }
                    }

                    // P2 possuindo ataca P1
                    if (p2.isPossessing) {
                        MonsterNode *m = (MonsterNode*)p2.possessedMonster;
                        if (m != NULL && m->data.state == MONSTER_ATTACK && p1.life > 0 && !p1.ghost) {

                            Vector2 monsterCenter = { m->data.position.x + 15, m->data.position.y + 15 };
                            float dist = Vector2Distance(monsterCenter, p1.position);

                            if (dist < (40.0f + 15.0f)) {
                                p2.life -= m->data.damage;
                                p1.damageCooldown = 1.0f;

                                if (p1.playerclass == CLASS_GUERREIRO) {
                                    p1.state = PLAYER_HURT;
                                    p1.currentFrame = 0;
                                    p1.frameTime = 0.0f;
                                }
                            }
                        }
                    }
                }
                // ===== fim dano possuído =====

                
                if (p1.life > 0 && !p1.ghost && p1.damageCooldown <= 0 && CheckPlayerHit(p1.position, 40.0f)) {
                    p1.life -= 20; 
                    p1.damageCooldown = 1.0f; 
                    if (p1.playerclass == CLASS_GUERREIRO) {
                        p1.state = PLAYER_HURT; 
                        p1.currentFrame = 0;    
                        p1.frameTime = 0.0f;
                    }
                }

                // PLAYER 2
                if (numPlayers == 2 && p2.life > 0 && !p2.ghost && p2.damageCooldown <= 0 && CheckPlayerHit(p2.position, 40.0f)) {
                    p2.life -= 20; 
                    p2.damageCooldown = 1.0f;
                    if (p2.playerclass == CLASS_GUERREIRO) {
                        p2.state = PLAYER_HURT;
                        p2.currentFrame = 0;
                        p2.frameTime = 0.0f;
                    }
                }

                // Trap
                if (numPlayers == 2) {
                    if (p1.ghost && p1.trapActive && !p2.ghost && Vector2Distance(p1.position, p2.position) < 100.0f) {
                        p2.life -= 80; 
                        p2.damageCooldown = 1.0f;
                        // Gatilho Hurt P2 (Opcional, já que 80 de dano quase mata)
                        if (p2.playerclass == CLASS_GUERREIRO) { p2.state = PLAYER_HURT; p2.currentFrame = 0; p2.frameTime = 0.0f; }
                    }
                    if (p2.ghost && p2.trapActive && !p1.ghost && Vector2Distance(p2.position, p1.position) < 100.0f) {
                        p1.life -= 80; 
                        p1.damageCooldown = 1.0f;
                        // Gatilho Hurt P1
                        if (p1.playerclass == CLASS_GUERREIRO) { p1.state = PLAYER_HURT; p1.currentFrame = 0; p1.frameTime = 0.0f; }
                    }
                }

                UpdateMonsters(&p1, &p2, numPlayers, &mapa); 
                UpdateProjectiles(&mapa, &p1);

                // ================================
                // IR PRO PRÓXIMO ANDAR (BOSS ROOM)
                // ================================
                Room *r = &mapa.dungeon.rooms[mapa.dungeon.currentRoom];

                if (r->type == ROOM_BOSS && r->portalActive && IsKeyPressed(KEY_E)) {

                    if (mapa.dungeon.floorLevel >= 5) {
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
                UnloadMapTextures();
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

                Rectangle src = { 0, 0, (float)texMenuBg.width, (float)texMenuBg.height };
                Rectangle dst = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };
                DrawTexturePro(texMenuBg, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);

                // logo
                float logoScale = 0.3f; 
                float logoW = texMenuLogo.width * logoScale;
                float logoH = texMenuLogo.height * logoScale;

                Vector2 logoPos = {
                    (GetScreenWidth()  - logoW) / 2.0f,
                    40.0f
                };

                DrawTextureEx(texMenuLogo, logoPos, 0.0f, logoScale, WHITE);

            
                const char* options[] = {
                    "NOVO JOGO",
                    "CARREGAR JOGO",
                    "SAIR"
                };

                float fontSize = 32.0f;
                float spacing  = 2.0f;
                float startY   = 290.0f;   // altura da primeira opção
                float stepY    = 50.0f;    // distância entre opções

                for (int i = 0; i < 3; i++) {
                    Color c = (i == mainMenuSelection) ? YELLOW : GRAY;

                    // centralizar o texto
                    Vector2 textSize = MeasureTextEx(fontMenu, options[i], fontSize, spacing);
                    Vector2 textPos = {
                        (GetScreenWidth() - textSize.x) / 2.0f,
                        startY + i * stepY
                    };

                    DrawTextEx(fontMenu, options[i], textPos, fontSize, spacing, c);

                    // Triângulo de seleção à esquerda (adaptado do seu)
                    if (i == mainMenuSelection) {
                        DrawTriangle(
                            (Vector2){ textPos.x - 20,              textPos.y + fontSize/2.0f },
                            (Vector2){ textPos.x - 30,              textPos.y + fontSize/2.0f - 10 },
                            (Vector2){ textPos.x - 30,              textPos.y + fontSize/2.0f + 10 },
                            RED
                        );
                    }
                }
            }
            
            else if (currentScreen == SCREEN_SLOT_SELECT) {
                // Fundo 
                Rectangle src = { 0, 0, (float)texMenuBg.width, (float)texMenuBg.height };
                Rectangle dst = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };
                DrawTexturePro(texMenuBg, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);

                // título 
                const char *title = isSavingMode ? "SALVAR JOGO" : "CARREGAR JOGO";
                float titleSize = 32.0f;
                float titleSpacing = 2.0f;
                Vector2 tSize = MeasureTextEx(fontMenu, title, titleSize, titleSpacing);
                DrawTextEx(fontMenu, title,
                        (Vector2){ (GetScreenWidth() - tSize.x)/2, 40 },
                        titleSize, titleSpacing, WHITE);

                // Slots
                for (int i = 0; i < 3; i++) {
                    int posX = 120 + i * 220;  
                    Rectangle slotRect = (Rectangle){ posX,  150, 180, 130 };

                    bool exists = SaveExists(i);
                    SaveData data = (SaveData){0};
                    if (exists) data = LoadGameData(i);

                    bool selected = (i == saveSlotSelection);
                    Color bg     = selected ? (Color){ 200, 50, 50, 180 } : (Color){ 0, 0, 0, 160 };
                    Color border = selected ? YELLOW : GRAY;

                    DrawRectangleRec(slotRect, bg);
                    DrawRectangleLinesEx(slotRect, 2.0f, border);

                    // "SLOT X"
                    DrawTextEx(fontMenu, TextFormat("SLOT %d", i + 1),
                            (Vector2){ slotRect.x + 10, slotRect.y + 10 },
                            20.0f, 2.0f, WHITE);

                    if (exists) {
                        DrawTextEx(fontMenu, data.saveName,
                                (Vector2){ slotRect.x + 10, slotRect.y + 40 },
                                16.0f, 1.0f, WHITE);
                        DrawTextEx(fontMenu, data.dateBuffer,
                                (Vector2){ slotRect.x + 10, slotRect.y + 60 },
                                11.0f, 1.0f, LIGHTGRAY);
                        DrawTextEx(fontMenu, TextFormat("Pts: %d", data.score),
                                (Vector2){ slotRect.x + 10, slotRect.y + 80 },
                                11.0f, 1.0f, YELLOW);
                    } else {
                        DrawTextEx(fontMenu, "VAZIO",
                                (Vector2){ slotRect.x + 40, slotRect.y + 55 },
                                20.0f, 2.0f, GRAY);
                    }
                }
            }


            else if (currentScreen == SCREEN_NAME_INPUT) {
                DrawTextEx(fontMenu, "DIGITE O NOME DO SAVE",
                (Vector2){ 100, 100 },
                30.0f, 2.0f, WHITE);
                
            
                DrawRectangle(250, 200, 300, 50, DARKGRAY);
                DrawRectangleLines(250, 200, 300, 50, WHITE);
                
                DrawText(tempName, 260, 210, 30, YELLOW);
                
                // Cursor Piscante
                framesCounter++;
                if ((framesCounter/30)%2 == 0) {
                    DrawText("_", 260 + MeasureText(tempName, 30), 210, 30, YELLOW);
                }
                
                DrawTextEx(fontMenu, "ENTER para confirmar",
                (Vector2){ 220, 300 },
                20.0f, 2.0f, GRAY);
            }

            else if (currentScreen == SCREEN_STORY) {
                // Fundo de historia
                Rectangle src = { 0, 0, (float)texStoryBg.width, (float)texStoryBg.height };
                Rectangle dst = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };
                DrawTexturePro(texStoryBg, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);

                // Caixa de texto
                Rectangle box = { 40, 300, 720, 140 };
                DrawRectangleRec(box, (Color){ 0, 0, 0, 180 });
                DrawRectangleLinesEx(box, 2.0f, DARKGRAY);

                // Pega só os caracteres já "revelados"
                const char *full = storyLines[storyCurrentLine];
                int len = (int)strlen(full);
                if (storyCharIndex > len) storyCharIndex = len;

                char buffer[512];
                if (storyCharIndex > 0) {
                    memcpy(buffer, full, storyCharIndex);
                }
                buffer[storyCharIndex] = '\0';

                DrawTextEx(fontMenu, buffer,
                        (Vector2){ box.x + 20, box.y + 20 },
                        20.0f, 2.0f, WHITE);

                DrawTextEx(fontMenu,
                        storyLineFinished ? "ENTER OU ESPACO para continuar" : "ENTER/ESPACO para acelerar",
                        (Vector2){ box.x + 20, box.y + box.height - 30 },
                        12.0f, 1.0f, YELLOW);
            }


            else if (currentScreen == SCREEN_CONFIRM_OVERWRITE) {
                DrawTextEx(fontMenu, "ATENCAO!",(Vector2){300, 100 }, 24.0f, 2.0f, RED);
                if (isSavingMode) DrawTextEx(fontMenu,"Deseja SOBRESCREVER este save?",
                (Vector2){ 150, 200 }, 20.0f, 2.0f, WHITE);
                else DrawTextEx(fontMenu,"Deseja CARREGAR este save?", 
                (Vector2){ 150, 250 }, 20.0f, 2.0f, WHITE);
                
                DrawTextEx(fontMenu,"[S] SIM    [N] NAO", (Vector2){ 250, 300 }, 16.0f, 2.0f, YELLOW);
            }

            else if (currentScreen == SCREEN_MSG_EMPTY) {
                DrawTextEx(fontMenu, "SLOT VAZIO!",(Vector2){ 200, 200 }, 30.0f, 2.0f, RED);
                DrawTextEx(fontMenu,"Pressione ENTER",(Vector2){ 220, 250 }, 20.0f, 2.0f, GRAY);
            }
            
            else if (currentScreen == SCREEN_PAUSE) {
                DrawRectangle(0,0,screenWidth, screenHeight, (Color){0,0,0,200});
                DrawTextEx(fontMenu,"PAUSA", (Vector2){ 300, 50 }, 40.0f, 2.0f, WHITE);
                const char* pOptions[] = {"VOLTAR", "SALVAR JOGO", "CARREGAR JOGO", "SAIR PARA MENU"};
                for (int i = 0; i < 4; i++) {
                    Color c = (i == pausemenu) ? YELLOW : GRAY;
                    DrawTextEx(fontMenu, pOptions[i],
                    (Vector2){ 300, 150 + i*50 },
                    20.0f, 2.0f, c);
                    if (i == pausemenu) DrawText(">", 280, 150 + i*50, 20, RED);
                }
            }
            else if (currentScreen == SCREEN_NUM_PLAYERS) {
                int opt1X = 120; int opt2X = 460; int optY = 200;  
                DrawTextEx(fontMenu, "QUANTOS PLAYERS?", 
                (Vector2){ 170, 100 }, 30.0f, 2.0f,WHITE);
                DrawTextEx(fontMenu, "1 PLAYER",
                (Vector2){ opt1X, optY },
                30.0f, 2.0f,
                (numPlayers == 1) ? YELLOW : GRAY);

                DrawTextEx(fontMenu, "2 PLAYERS",
                (Vector2){ opt2X, optY },
                30.0f, 2.0f,
                (numPlayers == 2) ? YELLOW : GRAY);
                int arrowX = (numPlayers == 1) ? opt1X : opt2X;
                DrawTriangle((Vector2){arrowX - 20, optY + 15}, (Vector2){arrowX - 35, optY}, (Vector2){arrowX - 35, optY + 30}, RED);
            }
            else if (currentScreen == SCREEN_CLASS_SELECT) {
                DrawTextEx(fontMenu, "SELECAO DE CLASSE",
                (Vector2){ 140, 50 },
                30.0f, 2.0f, WHITE);
                DrawTextEx(fontMenu, "P1",
                (Vector2){ 150, 150 },
                30.0f, 2.0f, BLUE);
                DrawTextEx(fontMenu,
                p1.playerclass == CLASS_MAGO ? "MAGO" : "GUERREIRO",
                (Vector2){ 150, 200 },
                20.0f, 2.0f, YELLOW);
                if (p1.ready) {
                    DrawTextEx(fontMenu, "OK",
                    (Vector2){ 150, 250 },
                    20.0f, 2.0f, GREEN);
                }

                if (numPlayers == 2) {
                    DrawTextEx(fontMenu, "P2",
                    (Vector2){ 550, 150 },
                    30.0f, 2.0f, GREEN);

                    DrawTextEx(fontMenu,
                    p2.playerclass == CLASS_MAGO ? "MAGO" : "GUERREIRO",
                    (Vector2){ 550, 200 },
                    20.0f, 2.0f, YELLOW);

                    if (p2.ready) {
                    DrawTextEx(fontMenu, "OK",
                    (Vector2){ 550, 250 },
                    20.0f, 2.0f, GREEN);
                    }
                }
            }
            else if (currentScreen == SCREEN_GAMEPLAY) {
                // ---------- ÁREA DO MUNDO (com câmera) ----------
                BeginMode2D(camera);

                    DrawMap(mapa);

                    if (shouldDrawPlayer(p1)) DrawPlayer(p1);
                    if (shouldDrawPlayer(p2)) DrawPlayer(p2);


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

                DrawText(TextFormat("FLOOR: %d / 5", mapa.dungeon.floorLevel), 340, 35, 20, RED);

                if (GetMonsterCount() == 0) {
                    Room *r = &mapa.dungeon.rooms[mapa.dungeon.currentRoom];

                    if (r->type == ROOM_BOSS) {
                        r->portalActive = true;
                        DrawText("PRESSIONE [E] PARA DESCER", 280, 400, 20, ORANGE);
                    }
                }
            }
            
            // --- DESENHO GAME OVER  ---
            else if (currentScreen == SCREEN_GAMEOVER) {
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 200}); 
                DrawTextEx(fontMenu, "GAME OVER",
                (Vector2){ 180, 150 },
                50.0f, 2.0f, RED);

                DrawTextEx(fontMenu,
                TextFormat("SCORE FINAL: %d", p1.score + p2.score),
                (Vector2){ 200, 250 },
                30.0f, 2.0f, YELLOW);

                DrawTextEx(fontMenu, "APERTE ESC PARA VOLTAR",
                (Vector2){ 180, 350 },
                20.0f, 2.0f, GRAY);

            }
            
            else if (currentScreen == SCREEN_OPTIONS) DrawText("OPCOES", 350, 200, 20, WHITE);
            else if (currentScreen == SCREEN_CREDITS) {
                DrawTextEx(fontMenu,
                "AFINAL, VOCE CONSEGUIU O QUE ALMEJAVA\nMAS ISSO PODE TER CUSTADO\nMAIS DO QUE VOCE IMAGINAVA...",
                (Vector2){ 80, 200 },
                16.0f,                   
                2.0f,                    
                WHITE);    
                DrawTextEx(fontMenu,
                "FIM.",
                (Vector2){ 350, 300 },
                16.0f,                   
                2.0f,                    
                RED);               
            }

        EndDrawing();
    }

    UnloadTexture(texMenuBg);
    UnloadTexture(texMenuLogo);
    UnloadTexture(texStoryBg); 
    UnloadMonsters();
    UnloadProjectiles();
    StopMusicStream(soundtrack);
    UnloadMusicStream(soundtrack);
    CloseAudioDevice();
    CloseWindow(); 
    return 0;
}