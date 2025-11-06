#include "raylib.h"

//ESTRUTURAS BASE E TIPOS **************************************************************

typedef enum MonsterType{ // tipos de monstro, se for adicionar um novo monstro tem de por um desses aqui
    MONSTER_SLIME,
    MONSTER_SKELETON,
    MONSTER_ZOMBIE
}MonsterType


typedef struct Player{ // jogador, aqui ficam suas caracteristicas
    Vector2 position; // posição do jogador
    float speed;      // velocidade de movimento
    int life;         // vida
    Color color;      // cor
} Player;

typedef struct Monster{ //qui são as caracteristicas base do tipo "Monstro", são o que vai ser editado par adiferenciar cada um, dps agnt muda isso para adicionar sprites e animações
    Vector2 position;
    MonsterType type;
    int life;
    Color color;
}Monster;

typedef struct Projectil{ //caracteristicas dos projeteis, msm historia
    Vector2 position;
    Vector2 direction;
    float speed;
    int active;
}Projectil;

// FUNÇÕES DO JOGADOR************************************************************

void UpdatePlayer(Player *p) { //aqui são funções do jogador, por agora to pondo só movimento basico
    if (IsKeyDown(KEY_W)) p->position.y -= p->speed;
    if (IsKeyDown(KEY_S)) p->position.y += p->speed;
    if (IsKeyDown(KEY_A)) p->position.x -= p->speed;
    if (IsKeyDown(KEY_D)) p->position.x += p->speed;
}

void DrawPlayer(Player p) { // faz aparecer na tela, no momento ele vai ser essa bola msm
    DrawCircleV(p.position, 12, p.color);
}

// FUNÇÕES DOS MONSTROS *********************************************************

void DrawMonster(Monster m) {
    DrawCubeV(m.position, 15, m.color)
}

// --------------------------------------------------------------------------------
// PROGRAMA PRINCIPAL (MAIN) ******************************************************

int main(void)
{
    // 1. Configuração da Janela
    const int screenWidth = 800;
    const int screenHeight = 450;

    // InitWindow cria a janela do jogo
    InitWindow(screenWidth, screenHeight, "Hellshift - Teste");

    SetTargetFPS(60); // Define o FPS (Frames Per Second)

    // Criar jogador
    Player player = {
        .position = (Vector2){screenWidth/2, screenHeight/2},
        .speed = 3.0f,
        .life = 5,
        .color = BLUE
    };

    //Criar monstro
    Monster enemy = {
        .position = (Vector2){200, 200},
        .type = MONSTER_SLIME,
        .life = 3,
        .color = RED
    };

    // 2. Loop Principal do Jogo
    while (!WindowShouldClose())    // Executa enquanto o usuário não fechar a janela
    {
        // --- ATUALIZAÇÃO/UPDATE (Lógica do Jogo) ---
        UpdatePlayer(&player);

        // --- DESENHO/DRAW (O que aparece na tela) ---
        BeginDrawing();

            ClearBackground((WHITE){30, 30, 40, 255}); // Limpa a tela com a cor branca

            DrawText("Hellshift - teste", 190, 200, 20, LIGHTGRAY);

            DrawPlayer(player);
            DrawMonster(enemy);

        EndDrawing();
    }

    // 3. Finalização
    CloseWindow(); // Fecha a janela e libera os recursos

    return 0;
}