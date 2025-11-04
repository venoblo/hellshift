#include "raylib.h"

typedef enum monstertype{
     

}


typedef struct player{


} player;

typedef struct monster{

}monster;

typedef struct projectil{

}projectil;

int main(void)
{
    // 1. Configuração da Janela
    const int screenWidth = 800;
    const int screenHeight = 450;

    // InitWindow cria a janela do jogo
    InitWindow(screenWidth, screenHeight, "Meu Jogo Raylib [Mac/Linux]");

    SetTargetFPS(60); // Define o FPS (Frames Per Second)

    // 2. Loop Principal do Jogo
    while (!WindowShouldClose())    // Executa enquanto o usuário não fechar a janela
    {
        // --- ATUALIZAÇÃO (Lógica do Jogo) ---
        // (Seu código de lógica vai aqui)


        // --- DESENHO (O que aparece na tela) ---
        BeginDrawing();

            ClearBackground(RAYWHITE); // Limpa a tela com a cor branca

            DrawText("Parabéns! Sua primeira janela!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
    }

    // 3. Finalização
    CloseWindow(); // Fecha a janela e libera os recursos

    return 0;
}