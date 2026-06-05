#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "Player.h"
#include "GlobalVariables.h"
#include "ResourceManager.h"
#include "Input.h"
#include "Utils.h"
#include "Colors.h"
#include "DefinitionJulia.h" // Precisamos disso para acessar o enum AcaoBot

extern bool drawPlayerAsFish;

Player * createPlayer(void){   // creates the player with the inicial settings

    Player *p = (Player*) malloc(sizeof(Player));
    if (p == NULL) {
        return NULL;
    }

    p->collision.width = 30;
    p->collision.height = 20;
    p->collision.x = (globalPixelWidth - p->collision.width) / 2.0f ;
    p->collision.y = globalPixelHeight * 0.6f;
    p->realPos.x = p->collision.x;
    p->realPos.y = p->collision.y;
    p->prevPos.x = p->collision.x;
    p->prevPos.y = p->collision.y;
    p->oxygen = MAX_OXYGEN;
    p->damageCooldown = 0;
    p->speed.x = 120;
    p->speed.y = 120;
    p->netTimer = 0;
    p->netOffset = 38;
    p->netSize = (Vector2){24, 24};
    p->lastDir = DIR_RIGHT;

    p->key = (PlayerKeybind){
        .moveUp = KEY_W,
        .moveLeft = KEY_A,
        .moveDown = KEY_S,
        .moveRight = KEY_D,
        .capture = KEY_SPACE,
    };

    return p;
}

void drawPlayer(Player *p, float alpha, float animTime){
    Texture2D* texture = drawPlayerAsFish ? &rm.animalArray[1] : &rm.player;

    int res = drawPlayerAsFish? 16 : 64;
    Rectangle source = {res * (int)(animTime * 10), res * (p->collision.y == globalWaterSurfaceHeight), drawPlayerAsFish ? -res : res, res};

    if(p->netTimer > 0) {
        texture = &rm.playerAttacking;
        res = 128;
        source = (Rectangle){res * (int)((0.4 - p->netTimer) * 15), 0, res, res};
    }

    Rectangle dest = {
        roundf(interpolateFloat(p->prevPos.x, p->collision.x, alpha) + p->collision.width / 2) * currentWindowScale,
        roundf(interpolateFloat(p->prevPos.y, p->collision.y, alpha) + p->collision.height / 2 + (drawPlayerAsFish ? 2 * cos(animTime * 6) : 0)) * currentWindowScale,
        source.width * currentWindowScale,
        source.height * currentWindowScale
    };

    Vector2 offset = {res / 2 * currentWindowScale, res / 2 * currentWindowScale};
    Color tint = {255, 255, 255, 255 * (1 - (drawPlayerAsFish ? 0 : (int)(p->damageCooldown * 15) % 2))};

    if(p->lastDir == DIR_LEFT) {
        source.width *= -1;
    }

    DrawTexturePro(*texture, source, dest, offset, 0, tint);

    /*
    //Temporary player collision display
    DrawRectangle(
        p->collision.x * currentWindowScale,
        p->collision.y * currentWindowScale,
        p->collision.width * currentWindowScale,
        p->collision.height * currentWindowScale,
        (Color){0, 255, 255, 63}
    );
    //Temporary net collision display

    Vector2 netPos = {
        p->collision.x + (p->collision.width - p->netSize.x) / 2,
        p->collision.y + (p->collision.height - p->netSize.y) / 2
    };

    switch(p->lastDir) {
        case LEFT:
            netPos.x -= p->netOffset;
            break;
        default:
            netPos.x += p->netOffset;
    }

    if(p->netTimer > 0 && p->netTimer < 0.25) {
        DrawRectangle(
            netPos.x * currentWindowScale,
            netPos.y * currentWindowScale,
            p->netSize.x * currentWindowScale,
            p->netSize.y * currentWindowScale,
            (Color){255, 255, 0, 63}
        );
    }
    */
}

// ... resto do seu código de criação e desenho do Player ...

void updatePlayer(Player *p, float delta, bool controladoPorIA, AcaoBot acaoIA) {
    // 1. Cooldown de dano e timer da rede (mantidos iguais)
    if(p->damageCooldown > 0) {
        p->damageCooldown = fmax(0, p->damageCooldown - delta);
    }
    if(p->netTimer > 0) {
        p->netTimer = fmax(0, p->netTimer - delta);
    }

    p->prevPos.x = p->collision.x;
    p->prevPos.y = p->collision.y;


    // ==========================================
    // 2. ABSTRAÇÃO DE INPUT (O Segredo da Integração)
    // ==========================================
    bool tentarCapturar = false;
    bool moverDireita = false;
    bool moverEsquerda = false;
    bool moverCima = false;
    bool moverBaixo = false;

    if (controladoPorIA) {
        // Se a IA está no controle, traduzimos o Enum da Julia para intenções de movimento
        if (acaoIA == ACAO_CAPTURAR) tentarCapturar = true;
        if (acaoIA == ACAO_FRENTE)   moverDireita = true;
        if (acaoIA == ACAO_TRAS)     moverEsquerda = true;
        if (acaoIA == ACAO_CIMA)     moverCima = true;
        if (acaoIA == ACAO_BAIXO)    moverBaixo = true;
    } else {
        // Se o jogador humano está no controle, lemos os eventos do teclado/controle
        tentarCapturar = consumeInputEvent(INPUT_P1_CAPTURE);
        moverDireita   = consumeInputEvent(INPUT_P1_MOVE_RIGHT);
        moverEsquerda  = consumeInputEvent(INPUT_P1_MOVE_LEFT);
        moverCima      = consumeInputEvent(INPUT_P1_MOVE_UP);
        moverBaixo     = consumeInputEvent(INPUT_P1_MOVE_DOWN);
    }


    // ==========================================
    // 3. APLICAÇÃO DA FÍSICA E REGRAS DO JOGO
    // ==========================================
    
    // Ação de Captura
    if(tentarCapturar) {
        if(p->netTimer == 0) p->netTimer = 0.4;
    }

    // Movimentação com base nas variáveis booleanas abstratas
    if(moverDireita){
        p->realPos.x += p->speed.x * delta;
        p->lastDir = DIR_RIGHT;
    }
    if(moverEsquerda){
        p->realPos.x -= p->speed.x * delta;
        p->lastDir = DIR_LEFT;
    }
    if(moverCima){
        p->realPos.y -= p->speed.y * delta;
    }
    if(moverBaixo){
        p->realPos.y += p->speed.y * delta;
    }

    // Restante das regras originais mantidas perfeitamente
    p->oxygen = fmin(p->oxygen, MAX_OXYGEN);
    p->realPos.x = fmin(fmax(0, p->realPos.x), globalPixelWidth - p->collision.width);
    p->realPos.y = fmin(fmax(globalWaterSurfaceHeight, p->realPos.y), globalFloorHeight - p->collision.height);

    p->collision.x = roundf(p->realPos.x);
    p->collision.y = roundf(p->realPos.y);
}

void drawOxygenBar(Player *p){
    int tankX = 8;
    int tankY = 8;

    int barHeight = 4;
    int barX = tankX + 6;
    int barY = tankY + 6;

    //Change color of the bar based on the oxygen level
    Color startColor;
    Color endColor;
    float lerpAmount = 0.0f;

    //Blending from green to yellowish orange
    if(p->oxygen > MAX_OXYGEN / 2) {
        startColor = PICO_8_ORANGE;
        endColor = PICO_8_GREEN;
        lerpAmount = p->oxygen / (MAX_OXYGEN / 2) - 1.0f;
    }
    //Blending from yellowish orange to red
    else {
        startColor = PICO_8_RED;
        endColor = PICO_8_ORANGE;
        lerpAmount = p->oxygen / (MAX_OXYGEN / 2);
    }

    Color finalColor = interpolateColor(startColor, endColor, lerpAmount);

    DrawRectangle(
        (barX - 1) * currentWindowScale,
        (barY - 1) * currentWindowScale,
        (MAX_OXYGEN + 2) * currentWindowScale,
        (barHeight + 2) * currentWindowScale,
        PICO_8_BROWNISH_BLACK
    );

    DrawRectangle(
        barX * currentWindowScale,
        barY * currentWindowScale,
        round(p->oxygen) * currentWindowScale,
        barHeight * currentWindowScale,
        finalColor
    );
    
    //Draw the the tank on top of the oxygen bar
    Rectangle source = {0, 0, rm.oxyTank.width, rm.oxyTank.height};
    Rectangle dest = {
        tankX * currentWindowScale,
        tankY * currentWindowScale,
        rm.oxyTank.width * currentWindowScale,
        rm.oxyTank.height * currentWindowScale
    };

    DrawTexturePro(rm.oxyTank, source, dest, (Vector2){0, 0}, 0, WHITE);
}