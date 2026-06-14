#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "Npc.h"
#include "Score.h"
#include "GlobalVariables.h"
#include "ResourceManager.h"
#include "Utils.h"

#define DEFAULT_REMOVAL_COUNTDOWN 0.5

extern bool hudVisible;
extern bool onlySpawnGarbage;

Npc* createNpc(float speed, NPCType type){ //creates the npc with the starting values
    Npc *n = (Npc*)malloc(sizeof(Npc));
    if (n == NULL) {
        return NULL;
    }

    n->type = type;
    n->speed.x = speed;
    n->speed.y = 0;

    if(n->type == NPC_ANIMAL) {
        n->variant = GetRandomValue(0, 16);

        //Determines the collision and capture score depending on the variant
        switch(n->variant) {
            case 10: n->collision = (Rectangle){0, 0, 8, 14}; n->captureScore = -3; break;
            case 11: n->collision = (Rectangle){0, 0, 14, 8}; n->captureScore = -3; break;
            case 12: n->collision = (Rectangle){0, 0, 16, 10}; n->captureScore = -5; break;
            case 13: n->collision = (Rectangle){0, 0, 24, 10}; n->captureScore = -5; break;
            case 14: n->collision = (Rectangle){0, 0, 24, 10}; n->captureScore = -5; break;
            case 15: n->collision = (Rectangle){0, 0, 24, 12}; n->captureScore = -5; break;
            case 16: n->collision = (Rectangle){0, 0, 8, 32}; n->captureScore = -8; break;
            default: n->collision = (Rectangle){0, 0, 8, 8}; n->captureScore = -3;
        }

        n->collisionOxygen = -10;
        n->captureOxygen = -20;
        n->waveAmplitude = GetRandomValue(1, 2);
    }
    else{ //NPC_GARBAGE
        n->variant = GetRandomValue(0, 10);
        n->collision.width = 8;
        n->collision.height = 8;

        n->collisionOxygen = -20;
        n->captureOxygen = 0;
        n->captureScore = 1;
        n->waveAmplitude = GetRandomValue(1, 5);
    }

    n->collision.x = globalPixelWidth + 16; //Padding so the textures don't immediately pop into view
    n->collision.y = GetRandomValue(globalWaterSurfaceHeight + 8, (globalFloorHeight - n->collision.height));
    n->realPos.x = n->collision.x;
    n->realPos.y = n->collision.y;
    n->prevPos.x = n->collision.x;
    n->prevPos.y = n->collision.y;
    n->spawnLocation = n->collision.y;
    n->waveTime = GetRandomValue(-5, 5);
    
    n->removeOnCollision = false;
    n->shouldBeRemoved = false;
    n->removalCountdown = DEFAULT_REMOVAL_COUNTDOWN;

    return n;
}

Npc* createBubble(float speed){ //creates a bubble with the starting values
    Npc *n = (Npc*)malloc(sizeof(Npc));
    if(n == NULL) {
        return NULL;
    }

    n->type = NPC_BUBBLE;

    n->collision.width = 16;
    n->collision.height = 16;
    n->speed.x = 0;
    n->speed.y = speed;

    n->collision.x = (int)GetRandomValue(globalPixelWidth / 2, globalPixelWidth - n->collision.width);
    n->collision.y = globalPixelHeight;
    n->realPos.x = n->collision.x;
    n->realPos.y = n->collision.y;
    n->prevPos.x = n->collision.x;
    n->prevPos.y = n->collision.y;
    n->spawnLocation = n->collision.x;
    n->waveTime = GetRandomValue(0, 5);
    n->waveAmplitude = GetRandomValue(2, 4);

    n->collisionOxygen = 25;
    n->captureOxygen = 0;
    n->captureScore = 0;
    
    n->removeOnCollision = true;
    n->shouldBeRemoved = false;
    n->removalCountdown = 0.1;

    return n;
}

void drawNpc(Npc* n, float alpha){ //draws the npc
    if(!n->shouldBeRemoved) {
        Texture2D *texture;

        if(n->type == NPC_GARBAGE) {
            texture = &rm.enemyArray[n->variant];
        }
        else { //NPC_ANIMAL
            texture = &rm.animalArray[n->variant];
        }

        Rectangle source = (Rectangle){0, 0, (float)texture->width, (float)texture->height};
        Rectangle dest = {
            roundf(interpolateFloat(n->prevPos.x, n->collision.x, alpha) + n->collision.width / 2) * currentWindowScale,
            roundf(interpolateFloat(n->prevPos.y, n->collision.y, alpha) + n->collision.height / 2) * currentWindowScale,
            source.width * currentWindowScale,
            source.height * currentWindowScale
        };
        Vector2 offset = (Vector2){texture->width / 2 * currentWindowScale, texture->height / 2 * currentWindowScale};

        DrawTexturePro(*texture, source, dest, offset, 0, WHITE);

        /*
        //Temporary collision display
        Color color;
        switch(n->type) {
            case NPC_ANIMAL:
                color = (Color){0, 255, 0, 127};
                break;

            case NPC_GARBAGE:
                color = (Color){255, 0, 0, 127};
                break;

            default:
                color = (Color){0, 0, 255, 127};
        }
        DrawRectangle(
            n->collision.x * currentWindowScale,
            n->collision.y * currentWindowScale,
            n->collision.width * currentWindowScale,
            n->collision.height * currentWindowScale,
            color
        );
        */
    }
    else {
        if(n->captureScore != 0 && hudVisible) {
            char points[12];
            Color color;

            if(n->captureScore > 0) {
                sprintf(points, "+%d", n->captureScore);
                color = GREEN;
            }
            else {
                sprintf(points, "%d", n->captureScore);
                color = RED;
            }

            Vector2 textOffset = MeasureTextEx(GetFontDefault(), points, 10, 1);
            textOffset.x = (int)(textOffset.x / 2);
            textOffset.y = (int)(textOffset.y / 2);

            int x = roundf((n->collision.x + n->collision.width / 2 - textOffset.x)) * currentWindowScale;
            int y = roundf((n->collision.y + n->collision.height / 2 - textOffset.y - (int)(10 * (DEFAULT_REMOVAL_COUNTDOWN - n->removalCountdown)))) * currentWindowScale;
            drawOutlinedText(points, x, y, 10 * currentWindowScale, color, BLACK);
        }
    }
}

void drawBubble(Npc* n, float alpha) { //draws the bubble
    Texture2D *texture;

    Rectangle source = {0, 0, 16, 16};
    Rectangle dest = {
        roundf(interpolateFloat(n->prevPos.x, n->collision.x, alpha) + n->collision.width / 2) * currentWindowScale,
        roundf(interpolateFloat(n->prevPos.y, n->collision.y, alpha) + n->collision.height / 2) * currentWindowScale,
        16 * currentWindowScale,
        16 * currentWindowScale
    };
    Vector2 offset = {8 * currentWindowScale, 8 * currentWindowScale};

    if(n->shouldBeRemoved) {
        texture = &rm.bubblePop;
        source = (Rectangle){16 * (int)(1 - 10 * n->removalCountdown), 0, 16, 16};
    }
    else {
        texture = &rm.bubbleIdle;
    }

    DrawTexturePro(*texture, source, dest, offset, 0, WHITE);

    /*Temporary collision display
    DrawRectangle(
        n->collision.x * currentWindowScale,
        n->collision.y * currentWindowScale,
        n->collision.width * currentWindowScale,
        n->collision.height * currentWindowScale,
        (Color){0, 0, 255, 127}
    );
    */
}

void updateNpc(Npc *n, float delta){ //update the npc position and state
    if(n->shouldBeRemoved) {
        n->removalCountdown -= delta;
    }
    else {
        n->prevPos.x = n->collision.x;
        n->prevPos.y = n->collision.y;

        n->waveTime += delta;

        if(n->type == NPC_BUBBLE) {
            //Bubble 
            n->realPos.y -= n->speed.y * delta;
            n->realPos.x = n->spawnLocation + (n->waveAmplitude * n->waveTime * cos(n->waveTime * 4)) - delta * 2;

            if(n->realPos.y < globalWaterSurfaceHeight) {
                n->shouldBeRemoved = true;
            }
        }
        else {
            //Animal or garbage
            n->realPos.x -= n->speed.x * delta;
            n->realPos.y = n->spawnLocation + (n->waveAmplitude * cos(n->waveTime * 2));
        }

        n->collision.x = roundf(n->realPos.x);
        n->collision.y = roundf(n->realPos.y);
    }
}
