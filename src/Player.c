#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "Player.h"
#include "GlobalVariables.h"
#include "ResourceManager.h"
#include "Input.h"
#include "Utils.h"

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

void updatePlayer(Player *p, float delta){
    //Damage cooldown (invincibility frames)
    if(p->damageCooldown > 0) {
        p->damageCooldown = fmax(0, p->damageCooldown - delta);
    }

    //Net attack timer
    if(p->netTimer > 0) {
        p->netTimer = fmax(0, p->netTimer - delta);
    }

    //Only use net if the timer is set to 0
    if(consumeInputEvent(INPUT_P1_CAPTURE)) {
        if(p->netTimer == 0) p->netTimer = 0.4;
    }

    p->prevPos.x = p->collision.x;
    p->prevPos.y = p->collision.y;

    //Player movement
    if(consumeInputEvent(INPUT_P1_MOVE_RIGHT)){
        p->realPos.x += p->speed.x * delta;
        p->lastDir = DIR_RIGHT;
    }
    if(consumeInputEvent(INPUT_P1_MOVE_LEFT)){
        p->realPos.x -= p->speed.x * delta;
        p->lastDir = DIR_LEFT;
    }
    if(consumeInputEvent(INPUT_P1_MOVE_UP)){
        p->realPos.y -= p->speed.y * delta;
    }
    if(consumeInputEvent(INPUT_P1_MOVE_DOWN)){
        p->realPos.y += p->speed.y * delta;
    }

    //Ensures the player's oxygen levels don't go past the limit
    p->oxygen = fmin(p->oxygen, MAX_OXYGEN);

    //Border collision
    p->realPos.x = fmin(fmax(0, p->realPos.x), globalPixelWidth - p->collision.width);
    p->realPos.y = fmin(fmax(globalWaterSurfaceHeight, p->realPos.y), globalFloorHeight - p->collision.height);

    //Aligns the collision to the pixel grid without losing position data
    p->collision.x = roundf(p->realPos.x);
    p->collision.y = roundf(p->realPos.y);
}

void drawOxygenBar(Player *p){
    int tankX = 8;
    int tankY = 8;

    int barHeight = 4;
    int barX = tankX + 6;
    int barY = tankY + 6;

    // Change color of the bar based on the oxygen levels
    Color startColor;
    Color endColor;
    float lerpAmount = 0.0f;

    //Blending from green to yellow
    if(p->oxygen > 50) {
        startColor = YELLOW;
        endColor = GREEN;
        lerpAmount = (p->oxygen - 50.0f) / 50.0f;
    } 
    //Blending from yellow to red
    else {
        startColor = RED;
        endColor = YELLOW;
        lerpAmount = p->oxygen / 50.0f;
    }

    Color finalColor = ColorLerp(startColor, endColor, lerpAmount);
        
    DrawRectangle(
        barX * currentWindowScale,
        barY * currentWindowScale,
        round(p->oxygen) * currentWindowScale,
        barHeight * currentWindowScale,
        finalColor
    );
    
    // Draw the the tank on top of the bar
    Rectangle source = {0, 0, rm.oxyTank.width, rm.oxyTank.height};
    Rectangle dest = {
        tankX * currentWindowScale,
        tankY * currentWindowScale,
        rm.oxyTank.width * currentWindowScale,
        rm.oxyTank.height * currentWindowScale
    };

    DrawTexturePro(rm.oxyTank, source, dest, (Vector2){0, 0}, 0, WHITE);
}