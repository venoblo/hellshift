#include "player.h"
#include "projectile.h"
#include "map.h"
#include "monster.h"
#include <stdlib.h>
#include "raymath.h"  


#define ANIM_SPEED 0.15f 
#define ATTACK_SPEED 0.1f
#define PLAYER_SCALE 3.0f

int GetPlayerMaxFrames(Player p) {
    switch (p.state) {
        case PLAYER_IDLE:       return 6; 
        case PLAYER_WALK:       return 8; 
        case PLAYER_HURT:       return 4; 
        case PLAYER_DEATH:      return 4; 
        case PLAYER_REBIRTH:    return 4; 
        case PLAYER_GHOST: return 6; 
        
        case PLAYER_ATTACK:
            // Diferencia frames de ataque por classe
            if (p.playerclass == CLASS_GUERREIRO) return 7;
            else return 6; 
            
        case PLAYER_SPECIAL:
            if (p.playerclass == CLASS_GUERREIRO) return 11;
            else return 6; // Mago usa 6 (placeholder)
            
        default: return 1;
    }
}

void InitPlayer(Player *p) {
    // limpar o warning
    InitPlayerClassStats(p);
}

void InitPlayerClassStats(Player *p) {
    p->damageCooldown = 0.0f;
    p->ghost = false;
    p->attackTimer = 0.0f; 

    p->originalColor = p->color;

    p->state = PLAYER_IDLE;
    p->currentFrame = 0;
    p->frameTime = 0.0f;
    p->facingDirection = 1; // virado para direita
    p->isAttacking = false;

    // Lógica SWITCH/CASE para aplicar as estatísticas
    if (p->playerclass == CLASS_GUERREIRO) {
        // Stats do Guerreiro (Tanque)
        p->maxLife = 100;
        p->life = 100;
        p->speed = 2.0f; // Mais lento

        p->texIdle    = LoadTexture("resources/characters/warriorp1/Knight-Idle.png");
        p->texWalk    = LoadTexture("resources/characters/warriorp1/Knight-Walk.png");
        p->texAttack  = LoadTexture("resources/characters/warriorp1/Knight-Attack.png");
        p->texSpecial = LoadTexture("resources/characters/warriorp1/Knight-Special.png");
        p->texHurt    = LoadTexture("resources/characters/warriorp1/Knight-Hurt.png");
        p->texDeath   = LoadTexture("resources/characters/warriorp1/Knight-Death.png");
    } 
    else if (p->playerclass == CLASS_MAGO) {
        // Stats do Mago
        p->maxLife = 80;
        p->life = 80;
        p->speed = 2.5f; 
        
        // --- CARREGAMENTO DAS SPRITES DO MAGO  ---
    
        p->texIdle    = LoadTexture("resources/characters/magop1/Wizard-Idle.png");
        p->texWalk    = LoadTexture("resources/characters/magop1/Wizard-Walk.png");
        p->texAttack  = LoadTexture("resources/characters/magop1/Wizard-Attack.png");
        p->texSpecial = LoadTexture("resources/characters/magop1/Wizard-Attack.png");         
        p->texHurt    = LoadTexture("resources/characters/magop1/Wizard-Hurt.png");
        p->texDeath   = LoadTexture("resources/characters/magop1/Wizard-Death.png");
    }
    p->trapActive = false;
    p->lastMoveDir = (Vector2){ 0.0f, -1.0f }; // setar a direção de início para cima

}

void UnloadPlayerTextures(Player *p) {
    if (p->texIdle.id > 0) UnloadTexture(p->texIdle);
    if (p->texWalk.id > 0) UnloadTexture(p->texWalk);
    if (p->texAttack.id > 0) UnloadTexture(p->texAttack);
    if (p->texSpecial.id > 0) UnloadTexture(p->texSpecial);
    if (p->texHurt.id > 0) UnloadTexture(p->texHurt);
    if (p->texDeath.id > 0) UnloadTexture(p->texDeath);
}

#define DESPOSSESS_PLAYER_RADIUS 150.0f 
//update
void UpdatePlayer(Player *p, Map *map, Player *other) { //aqui são funções do jogadoe
    
    p->trapActive = false;
    if (p->isPossessing && p->possessedMonster != NULL) {
        MonsterNode *monstro = (MonsterNode*)p->possessedMonster;

        bool moving = false;

        // Controla o monstro!
        if (IsKeyDown(p->keyUp))    { monstro->data.position.y -= monstro->data.speed; moving = true; }
        if (IsKeyDown(p->keyDown))  { monstro->data.position.y += monstro->data.speed; moving = true; }
        if (IsKeyDown(p->keyLeft))  { monstro->data.position.x -= monstro->data.speed; moving = true; monstro->data.facingDirection = -1; }
        if (IsKeyDown(p->keyRight)) { monstro->data.position.x += monstro->data.speed; moving = true; monstro->data.facingDirection = 1; }

        // Player segue o monstro
        p->position = monstro->data.position;
        p->ghost = true;

        // --- ATAQUE MANUAL DO POSSUÍDO ---
        // --- AÇÃO: despossuir se estiver longe do player vivo e da trap/bomba ---
        if (IsKeyPressed(p->keyAction) && monstro->data.state != MONSTER_DEATH) {


            bool nearOtherPlayer = false;
            if (other != NULL && other->life > 0 && !other->ghost) {
                float dPlayer = Vector2Distance(other->position, monstro->data.position);
                if (dPlayer < DESPOSSESS_PLAYER_RADIUS) nearOtherPlayer = true;
            }

            // Usa a própria checagem de trap do mapa como veto
            bool nearTrap = CheckTrapInteraction(map, monstro->data.position);

            if (!nearOtherPlayer && !nearTrap) {
                // despossui
                monstro->data.isPossessed = false;
                monstro->data.color = WHITE;
                monstro->data.state = MONSTER_IDLE;
                monstro->data.currentFrame = 0;
                monstro->data.frameTime = 0.0f;

                p->isPossessing = false;
                p->possessedMonster = NULL;
                p->ghost = true;
                p->state = PLAYER_GHOST;
                p->currentFrame = 0;
                p->frameTime = 0.0f;

                return; // sai da possessão neste frame
            }

            // se não pode despossuir, então é ataque normal
            if (monstro->data.attackCooldownTimer <= 0.0f) {
                monstro->data.state = MONSTER_ATTACK;
                monstro->data.currentFrame = 0;
                monstro->data.frameTime = 0.0f;
            }
        }


        // --- ANIMAÇÃO DO POSSUÍDO (porque UpdateOneMonster é pulado) ---
        float animSpeed = 0.15f;
        int maxF = 1;

        // baixa cooldown mesmo possuído
        if (monstro->data.attackCooldownTimer > 0) {
            monstro->data.attackCooldownTimer -= GetFrameTime();
            if (monstro->data.attackCooldownTimer < 0) monstro->data.attackCooldownTimer = 0;
        }

        if (monstro->data.state == MONSTER_ATTACK) {
            animSpeed = GetMonsterAttackAnimSpeedGeneric(&monstro->data);
            maxF = GetMonsterMaxFramesGeneric(&monstro->data, MONSTER_ATTACK);
        }
        else if (moving) {
            monstro->data.state = MONSTER_WALK;
            maxF = GetMonsterMaxFramesGeneric(&monstro->data, MONSTER_WALK);
        }
        else {
            monstro->data.state = MONSTER_IDLE;
            maxF = GetMonsterMaxFramesGeneric(&monstro->data, MONSTER_IDLE);
        }



        monstro->data.frameTime += GetFrameTime();
        if (monstro->data.frameTime >= animSpeed) {
            monstro->data.frameTime = 0.0f;
            monstro->data.currentFrame++;

            if (monstro->data.currentFrame >= maxF) {
                monstro->data.currentFrame = 0;

                // se terminou ataque, volta pra walk/idle
                if (monstro->data.state == MONSTER_ATTACK) {
                    monstro->data.state = moving ? MONSTER_WALK : MONSTER_IDLE;
                }
            }
        }

        // Se o monstro morrer, expulsa o jogador
        if (monstro->data.life <= 0) {
            monstro->data.isPossessed = false;   // libera flag

            p->isPossessing = false;
            p->possessedMonster = NULL;
            p->ghost = true;
            p->state = PLAYER_REBIRTH;
            p->currentFrame = 3;
        }

        return;
    }

    bool isMoving = false;
    Vector2 oldPos = p->position;

    bool inputBlocked = (p->state == PLAYER_HURT || p->state == PLAYER_DEATH || p->state == PLAYER_ATTACK || p->state == PLAYER_REBIRTH);    

    if (p->ghost) {

        float ghostSpeed = p->speed * 1.5f; 

        if (IsKeyDown(p->keyUp))    { p->position.y -= ghostSpeed; isMoving = true; } 
        if (IsKeyDown(p->keyDown))  { p->position.y += ghostSpeed; isMoving = true; }
        if (IsKeyDown(p->keyLeft))  { p->position.x -= ghostSpeed; isMoving = true; p->facingDirection = -1; } 
        if (IsKeyDown(p->keyRight)) { p->position.x += ghostSpeed; isMoving = true; p->facingDirection = 1; }  
        
        // Mantém o fantasma dentro da tela
        if (p->position.x < 0) p->position.x = 0;
        if (p->position.y < 0) p->position.y = 0;
        if (p->position.x > 800) p->position.x = 800;
        if (p->position.y > 450) p->position.y = 450;

        p->state = PLAYER_GHOST;

        if (IsKeyPressed(p->keyAction)) {
            // 1. Tenta Possuir
            MonsterNode *alvo = GetClosestMonsterNode(p->position, 50.0f);
            if (alvo != NULL) {
                p->isPossessing = true;
                p->possessedMonster = alvo;
                alvo->data.isPossessed = true;  
               //diferencia dos outros esqueletos
                alvo->data.color = ColorAlpha(DARKGRAY, 0.9f);
 
                return;
            }

            // Tenta Ativar Armadilha
            if (CheckTrapInteraction(map, p->position)) {
                p->trapActive = true;
                // "causando dano no jogador que está vivo" -> Ok!
            }
        }    
        return;
        
    }
    
    
    // movimentação jogadores vivos

    Vector2 move = { 0 };

    if (IsKeyDown(p->keyUp)) { 
        move.y -= p->speed;
        isMoving = true;
        p->lastMoveDir = (Vector2){ 0.0f, -1.0f };
    }
    if (IsKeyDown(p->keyDown)) { 
        move.y += p->speed;
        isMoving = true;
        p->lastMoveDir = (Vector2){ 0.0f, 1.0f };
    }
    if (IsKeyDown(p->keyLeft)) { 
        move.x -= p->speed;
        isMoving = true;
        p->facingDirection = -1;
        p->lastMoveDir = (Vector2){ -1.0f, 0.0f };
    }
    if (IsKeyDown(p->keyRight)) { 
        move.x += p->speed;
        isMoving = true;
        p->facingDirection = 1;
        p->lastMoveDir = (Vector2){ 1.0f, 0.0f };
    }

    Vector2 newPos = {
        p->position.x + move.x,
        p->position.y + move.y
    };

    if (!CheckMapCollision(*map, newPos)) {
        p->position = newPos;
    }


    // ação jogadores vivos
    if (IsKeyPressed(p->keyAction)) {

        p->state = PLAYER_ATTACK;
        p->currentFrame = 0;  
        p->frameTime = 0.0f;
        
        // 1. Ataque Mago (Projétil)
        if (p->playerclass == CLASS_MAGO) {
            Vector2 direcaoDoTiro = p->lastMoveDir;

            // segurança: se por algum bug vier zero, atira pra direita
            if (direcaoDoTiro.x == 0.0f && direcaoDoTiro.y == 0.0f) {
                direcaoDoTiro = (Vector2){1.0f, 0.0f};
            }

            SpawnProjectile(p->position, direcaoDoTiro);
        }

        // 2. Ataque Guerreiro (Melee)
        else if (p->playerclass == CLASS_GUERREIRO) {
            float alcanceEspada = 100.0f;
            int dano = 25; 
            
            // Chama a função que criamos no monster.c
            int pontos = CheckMeleeAttack(p->position, alcanceEspada, dano);
            
            p->score += pontos; // Soma a pontuação
            p->attackTimer = 0.2f; // Ativa o efeito visual
        }
    }
    bool actionInProgress = (p->state == PLAYER_ATTACK || p->state == PLAYER_HURT || p->state == PLAYER_DEATH || p->state == PLAYER_REBIRTH);

    if (!actionInProgress) { 
        if (isMoving) {
            p->state = PLAYER_WALK;
        } else {
            p->state = PLAYER_IDLE;
        }
    }
    
    // O "Ticker" (Relógio da Animação)
    p->frameTime += GetFrameTime();
    
    // Velocidade da animação
    float currentSpeed = ANIM_SPEED; 

    if (p->state == PLAYER_ATTACK) {
        currentSpeed = ATTACK_SPEED; 
    } 
    else if (p->state == PLAYER_HURT) {
        currentSpeed = 0.06f; 
    }

    if (p->frameTime >= currentSpeed) {
        p->frameTime = 0.0f;
        p->currentFrame++; // Passa para a próxima "foto"
        
        // Aqui precisamos saber o limite de frames.
        // Como simplificação, vamos pegar os valores do Guerreiro que você me passou:
        int maxFrames = 0;
        switch(p->state) {
            case PLAYER_IDLE: maxFrames = 6; break;
            case PLAYER_WALK: maxFrames = 8; break;
            case PLAYER_ATTACK: maxFrames = 7; break;
            case PLAYER_SPECIAL: maxFrames = 11; break; // Adicionei Special se tiver
            case PLAYER_HURT: maxFrames = 4; break;
            case PLAYER_DEATH: maxFrames = 4; break;
            case PLAYER_REBIRTH: maxFrames = 4; break;
            default: maxFrames = 4; break;
        }
        // Se passou do último frame
        if (p->currentFrame >= maxFrames) {
            
            // 1. SEQUÊNCIA DE MORTE
            if (p->state == PLAYER_DEATH) {
                p->state = PLAYER_REBIRTH; 
                p->currentFrame = 0; 
            }
            else if (p->state == PLAYER_REBIRTH) {
                p->ghost = true;        
                p->state = PLAYER_GHOST; 
                p->life = 0;            
                p->currentFrame = 0;
            }
            
            // 2. FIM DE ATAQUE OU DANO
            else if (p->state == PLAYER_ATTACK || p->state == PLAYER_HURT) {
                p->currentFrame = 0;
                
               
                if (p->life <= 0) {
                    p->state = PLAYER_DEATH;
                } else {
                    p->state = PLAYER_IDLE; // Só volta ao normal se tiver vida
                }
            }
            else {
                p->currentFrame = 0; // Loop normal
            }
        }
    }

    if (p->damageCooldown > 0) {
        p->damageCooldown -= GetFrameTime();
        if (p->state != PLAYER_HURT) {
            if (((int)(p->damageCooldown * 10)) % 2 == 0) p->color = p->originalColor;
            else p->color = RED;  
        } else {
            p->color = p->originalColor;
        }
    } else {
        p->color = p->originalColor;
    }

}

bool shouldDrawPlayer(Player p) {
    return (
        p.life > 0 ||
        p.ghost ||
        p.state == PLAYER_DEATH ||
        p.state == PLAYER_REBIRTH
    );
}




void DrawPlayer(Player p) { 
    if (p.isPossessing) return;

   // --- fantasma ---
    
    // --- GUERREIRO ---
    
    Texture2D texToDraw;
    int maxFrames = 1;
    const char* stateName = "UNKNOWN";

    int frameToDraw = p.currentFrame;


    switch (p.state) {
        case PLAYER_IDLE:       texToDraw = p.texIdle;    stateName = "IDLE"; break;
        case PLAYER_WALK:       texToDraw = p.texWalk;    stateName = "WALK"; break;
        case PLAYER_ATTACK:     texToDraw = p.texAttack;  stateName = "ATTACK"; break;
        case PLAYER_SPECIAL:    texToDraw = p.texSpecial; stateName = "SPECIAL"; break;
        case PLAYER_HURT:       texToDraw = p.texHurt;    stateName = "HURT"; break;
        case PLAYER_DEATH:      texToDraw = p.texDeath;   stateName = "DEATH"; break;
        case PLAYER_REBIRTH:    
            texToDraw = p.texDeath;   
            stateName = "REBIRTH";
            // flipar
            {
                int max = GetPlayerMaxFrames(p);

                frameToDraw = max - 1 - p.currentFrame;
                if (frameToDraw < 0) frameToDraw = 0;
                if (frameToDraw >= max) frameToDraw = max - 1;
                
            }
            break;
        case PLAYER_GHOST: texToDraw = p.texIdle;    stateName = "GHOST"; break;
        default:                texToDraw = p.texIdle;    break;
    }

    // --- VERIFICAÇÃO DE DEBUG (Corrigida) ---
    bool textureFailed = (texToDraw.id <= 0);
    
    // Desenha um quadrado de fundo SE estiver morrendo ou renascendo
    // Isso prova que o código entrou no estado certo
    if (p.state == PLAYER_DEATH || p.state == PLAYER_REBIRTH) {
        DrawRectangle(p.position.x - 20, p.position.y - 20, 40, 40, RED);
        DrawText(stateName, p.position.x, p.position.y - 50, 10, YELLOW);
    }

    if (textureFailed) {
        DrawRectangle(p.position.x - 20, p.position.y - 20, 40, 40, MAGENTA);
        DrawText("NO PNG", p.position.x - 30, p.position.y - 40, 10, RED);
        return;
    }

    int realMaxFrames = GetPlayerMaxFrames(p);
    float frameWidth = (float)texToDraw.width / realMaxFrames;
    Rectangle sourceRec = { 
        frameWidth * frameToDraw,  
        0.0f,                             
        frameWidth * p.facingDirection,   // multiplica por -1 negativa para flipar para esq
        (float)texToDraw.height           
    };

   
    Rectangle destRec = {
        p.position.x,
        p.position.y,
        frameWidth * PLAYER_SCALE,        
        (float)texToDraw.height * PLAYER_SCALE 
    };

    // hitbox
    Vector2 origin = { 
        destRec.width / 2.0f, 
        destRec.height / 2.0f
    };

    
    Color drawColor = WHITE;

    if (p.state == PLAYER_GHOST) {
    drawColor = ColorAlpha(WHITE, 0.6f);
    }

    else if (p.damageCooldown > 0 && p.state != PLAYER_HURT) {
        if (((int)(p.damageCooldown * 10)) % 2 != 0) {
            drawColor = RED;
        }
    }

    // DESENHO FINAL
    DrawTexturePro(texToDraw, sourceRec, destRec, origin, 0.0f, drawColor);

    DrawText(TextFormat("State: %s", stateName), p.position.x - 20, p.position.y - 60, 10, YELLOW);
    DrawText(TextFormat("Frame: %d/%d", frameToDraw, maxFrames), p.position.x - 20, p.position.y - 50, 10, YELLOW);
    DrawText(TextFormat("Life: %d", p.life), p.position.x - 20, p.position.y - 40, 10, GREEN);

    // DEBUG: Descomente abaixo para ver se a colisão bate com o desenho
    DrawCircleLines(p.position.x, p.position.y, 12, GREEN);
}