/**
 * @file GameWorld.h
 * @author Prof. Dr. David Buzatto
 * @brief GameWorld implementation.
 * 
 * @copyright Copyright (c) 2025
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"

#include "GameWorld.h"
#include "GameWindow.h"
#include "GlobalVariables.h"
#include "ResourceManager.h"
#include "Utils.h"
#include "Enums.h"
#include "Colors.h"

#include "Player.h"
#include "Npc.h"
#include "GameMechanics.h"
#include "Score.h"
#include "Input.h"

const int globalPixelWidth = 320;
const int globalPixelHeight = 180;
const int globalWaterSurfaceHeight = 60;
const int globalFloorHeight = 172;

bool hudVisible = true;
bool pauseOxygen = false;
bool randomSpeed = true;
bool drawPlayerAsFish = false;
bool onlySpawnGarbage = false;

//#include "raylib/raymath.h"
//#define RAYGUI_IMPLEMENTATION    // to use raygui, comment these three lines.
//#include "raylib/raygui.h"       // other compilation units must only include
//#undef RAYGUI_IMPLEMENTATION     // raygui.h

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld* createGameWorld(void) {
    GameWorld *gw = calloc(1, sizeof(*gw));
    gw->player = createPlayer();
    gw->spawnInterval = INITIAL_SPAWN_INTERVAL;
    gw->npcSpeed = 60;
    return gw;
}

/**
 * @brief Destroys a GameWorld object and its dependecies.
 */
void destroyGameWorld(GameWorld **gw) {
    if(gw == NULL || *gw == NULL) return;

    for(int i = 0; i < MAX_NPC; i++) {
        if((*gw)->npc[i] != NULL) free((*gw)->npc[i]);
    }
    free((*gw)->player);
    free(*gw);
    *gw = NULL;
}

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld(void *gameWorld, float delta, void* additionalData) {
    GameWorld *gw = (GameWorld*)gameWorld;

    //Pause function
    if(consumeInputEvent(INPUT_UI_ESCAPE)) {
        setGameState(additionalData, GAME_PAUSED);
    }
    else {
        gw->previousTime = gw->gameTime;
        gw->gameTime += delta;
        gw->spawnTimer += delta;
        gw->bubbleTimer += delta;
        
        updatePlayer(gw->player, delta);

        //Npc spawn logic
        if(gw->spawnTimer > gw->spawnInterval){
            gw->spawnTimer = 0.0f;

            if(gw->activeNpc < MAX_NPC) {
                for (int i = 0; i < MAX_NPC; i++) {
                    if (gw->npc[i] == NULL) {
                        int npcSpeed = randomSpeed ? fmin(gw->npcSpeed + GetRandomValue(0, 100), MAX_NPC_SPEED) : gw->npcSpeed;
                        gw->npc[i] = createNpc(npcSpeed);
                        gw->activeNpc++;
                        break;                   
                    }
                }
            }
        }

        //Bubble spawn logic
        if(gw->bubbleTimer > BUBBLE_SPAWN_INTERVAL){
            gw->bubbleTimer = 0.0f;

            if(gw->activeNpc < MAX_NPC) {
                for(int i = 0; i < MAX_NPC; i++) {
                    if(gw->npc[i] == NULL) {
                        gw->npc[i] = createBubble(68);
                        gw->activeNpc++;
                        break;
                    }
                }
            }
        }

        //Increase the speed and decrease spawn interval of the game if x enemies escape/are captured
        if(gw->escapedEnemies >= ENEMY_ESCAPE_LIMIT || gw->caughtEnemies >= ENEMY_CAUGHT_LIMIT) {
            gw->npcSpeed = fmin(gw->npcSpeed + 4, MAX_NPC_SPEED);
            gw->spawnInterval = fmax(MIN_SPAWN_INTERVAL, gw->spawnInterval - SPAWN_DECREMENT);

            //Reset the counters
            gw->escapedEnemies = 0;
            gw->caughtEnemies = 0;
        }

        //Update all NPCs
        for(int i = 0; i < MAX_NPC; i++) {
            //Only update NPC if it is currently active
            if(gw->npc[i] != NULL) {
                updateNpc(gw->npc[i], delta);

                //Checks if the NPC is marked for removal
                if(gw->npc[i]->shouldBeRemoved) {
                    //Checks if the NPC should be removed in the current frame
                    if(gw->npc[i]->removalCountdown <= 0) {
                        free(gw->npc[i]);
                        gw->npc[i] = NULL;
                        gw->activeNpc--;
                        continue;
                    }
                }
                else {
                    checkNpcCapture(gw, gw->player, gw->npc[i]);
                    checkNpcCollision(gw->player, gw->npc[i]);

                    //Checks if the NPC has escaped
                    if(gw->npc[i]->collision.x + gw->npc[i]->collision.width + 16 < 0){
                        gw->npc[i]->shouldBeRemoved = true;
                        gw->npc[i]->removalCountdown = 0;

                        if(gw->npc[i]->type == NPC_GARBAGE) {
                            gw->escapedEnemies++;
                        }
                    }
                }
            }
        }

        //Oxygen control
        if(!pauseOxygen) {
            if(gw->player->oxygen > 0){
                if(gw->player->collision.y == globalWaterSurfaceHeight && gw->player->netTimer == 0) {
                    if(gw->player->oxygen < MAX_OXYGEN) {
                        gw->player->oxygen = fmin(gw->player->oxygen + 6 * delta, MAX_OXYGEN);
                    }
                }
                else if(gw->player->collision.y + gw->player->collision.height / 2 < (globalPixelHeight * 2 / 3)) {
                    gw->player->oxygen -= 3 * delta;
                }
                else {
                    gw->player->oxygen -= 6 * delta;
                }
            }
            else{
                updateBestScore();
                setGameState(additionalData, GAME_OVER);
            }
        }
    }
}


/**
 * @brief Draws the state of the game.
 */
void drawGameWorld(void *gameWorld, float alpha, void* additionalData) {
    GameWorld *gw = (GameWorld*)gameWorld;

    float animTime = interpolateFloat(gw->previousTime, gw->gameTime, alpha);

    drawBackground(animTime);

    drawPlayer(gw->player, alpha, animTime);

    for (int i = 0; i < MAX_NPC; i++) {
        if(gw->npc[i] != NULL) {
            if(gw->npc[i]->type == NPC_BUBBLE) {
                drawBubble(gw->npc[i], alpha);
            }
            else {
                drawNpc(gw->npc[i], alpha);
            }
        }
    }

    drawForeground(animTime);
    
    //Water surface height
    //DrawLine(0, globalWaterSurfaceHeight * currentWindowScale, GetScreenWidth(), globalWaterSurfaceHeight * currentWindowScale, BLUE);

    if(hudVisible) {
        drawOxygenBar(gw->player);
        drawInGameScore();
    }
}

/**
 * @brief Draws the environment behind all entities.
 */
void drawBackground(float animTime) {
    Texture2D *celestial;
    Color skyColor;
    Color cloudShadowColor;
    Color cloudHighlightColor;
    Color cityscapeColor;
    Color cityOverlayColor;
    Color waterColor;

    int dayTimeInterval = 60;

    //Used for updating the color of certain background elements
    float colorLerpProgress = fmin(fmax(0, fmod(animTime, dayTimeInterval) - dayTimeInterval + 1), 1);

    switch((int)(animTime / dayTimeInterval) % 4) {
        case 1: //Sunset
            celestial = &rm.sunBg;
            skyColor = interpolateColor(PICO_8_DARK_ORANGE, PICO_8_DARK_BLUE, colorLerpProgress);
            cloudHighlightColor = interpolateColor(PICO_8_WHITE, PICO_8_LIGHT_GREY, colorLerpProgress);
            cloudShadowColor = interpolateColor(PICO_8_LIGHT_PEACH, PICO_8_LAVENDER, colorLerpProgress);
            cityscapeColor = interpolateColor(PICO_8_DARKER_GREY, PICO_8_BLACK, colorLerpProgress);
            cityOverlayColor = interpolateColor(PICO_8_DARKER_GREY, PICO_8_LIGHT_YELLOW, colorLerpProgress);
            waterColor = interpolateColor(PICO_8_BLUE_GREEN, PICO_8_DARKER_BLUE, colorLerpProgress);
            break;

        case 2: //Night
            celestial = &rm.moonBg;
            skyColor = interpolateColor(PICO_8_DARK_BLUE, PICO_8_MAUVE, colorLerpProgress);
            cloudHighlightColor = interpolateColor(PICO_8_LIGHT_GREY, PICO_8_PEACH, colorLerpProgress);
            cloudShadowColor = interpolateColor(PICO_8_LAVENDER, PICO_8_DARK_PEACH, colorLerpProgress);
            cityscapeColor = interpolateColor(PICO_8_BLACK, PICO_8_DARKER_GREY, colorLerpProgress);
            cityOverlayColor = PICO_8_LIGHT_YELLOW;
            waterColor = interpolateColor(PICO_8_DARKER_BLUE, PICO_8_BLUE_GREEN, colorLerpProgress);
            break;

        case 3: //Sunrise
            celestial = &rm.moonBg;
            skyColor = interpolateColor(PICO_8_MAUVE, PICO_8_BLUE, colorLerpProgress);
            cloudHighlightColor = interpolateColor(PICO_8_PEACH, PICO_8_WHITE, colorLerpProgress);
            cloudShadowColor = interpolateColor(PICO_8_DARK_PEACH, PICO_8_LIGHT_GREY, colorLerpProgress);
            cityscapeColor = interpolateColor(PICO_8_DARKER_GREY, PICO_8_TRUE_BLUE, colorLerpProgress);
            cityOverlayColor = interpolateColor(PICO_8_LIGHT_YELLOW, PICO_8_TRUE_BLUE, colorLerpProgress);
            waterColor = PICO_8_BLUE_GREEN;
            break;

        default:
            celestial = &rm.sunBg;
            skyColor = interpolateColor(PICO_8_BLUE, PICO_8_DARK_ORANGE, colorLerpProgress);
            cloudHighlightColor = PICO_8_WHITE;
            cloudShadowColor = interpolateColor(PICO_8_LIGHT_GREY, PICO_8_LIGHT_PEACH, colorLerpProgress);
            cityscapeColor = interpolateColor(PICO_8_TRUE_BLUE, PICO_8_DARKER_GREY, colorLerpProgress);
            cityOverlayColor = interpolateColor(PICO_8_TRUE_BLUE, PICO_8_DARKER_GREY, colorLerpProgress);
            waterColor = PICO_8_BLUE_GREEN;
    }

    Rectangle source;
    Rectangle dest;
    Vector2 offset = {0, 0};

    //Sky
    DrawRectangle(0, 0, GetScreenWidth(), 64 * currentWindowScale, skyColor);

    //Celestial
    float celestialAngle = (fmod((animTime + 1) / dayTimeInterval / 2, 1)) * PI;
    int celestialX = (int)(globalPixelWidth * (0.5 - 0.4 * cosf(celestialAngle))) * currentWindowScale;
    int celestialY = (int)(globalPixelHeight * (0.5 - 0.3 * sinf(celestialAngle))) * currentWindowScale;
    source = (Rectangle){0, 0, 32, 32};
    dest = (Rectangle){celestialX, celestialY, 32 * currentWindowScale, 32 * currentWindowScale};
    DrawTexturePro(*celestial, source, dest, (Vector2){16 * currentWindowScale, 24 * currentWindowScale}, 0, WHITE);

    //Clouds
    source = (Rectangle){0, 0, 320, 64};
    dest = (Rectangle){0, 0, GetScreenWidth(), 64 * currentWindowScale};
    DrawTexturePro(rm.cloudShadowBg, source, dest, offset, 0, cloudShadowColor);
    DrawTexturePro(rm.cloudHighlightBg, source, dest, offset, 0, cloudHighlightColor);

    //Cityscape
    source = (Rectangle){roundf(animTime), 0, 320, 180};
    dest = (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()};
    DrawTexturePro(rm.cityscapeBg, source, dest, offset, 0, cityscapeColor);
    DrawTexturePro(rm.cityOverlayBg, source, dest, offset, 0, cityOverlayColor);

    //Water
    dest = (Rectangle){0, 52 * currentWindowScale, GetScreenWidth(), 128 * currentWindowScale};
    source = (Rectangle){320 + roundf(animTime * 70), 0, 320, 128};
    DrawTexturePro(rm.waterBg, source, dest, offset, 0, waterColor);
    source.height /= 4;
    dest.height /= 4;
    DrawTexturePro(rm.foamFg, source, dest, offset, 0, WHITE);
    source = (Rectangle){roundf(animTime * 90), 0, 320, 128};
    dest.height *= 4;
    DrawTexturePro(rm.waterBg, source, dest, offset, 0, waterColor);

    //Floor
    source = (Rectangle){roundf(animTime * 40), 0, 320, 64};
    dest = (Rectangle){0, 116 * currentWindowScale, GetScreenWidth(), 64 * currentWindowScale};
    DrawTexturePro(rm.floorBg, source, dest, offset, 0, WHITE);
}

/**
 * @brief Draws the environment in front of all entities.
 */
void drawForeground(float animTime) {
    Rectangle source;
    Rectangle dest;
    Vector2 offset = {0, 0};

    //Foam
    source = (Rectangle){roundf(animTime * 90), 0, 320, 32};
    dest = (Rectangle){0, 52 * currentWindowScale, GetScreenWidth(), 32 * currentWindowScale};
    DrawTexturePro(rm.foamFg, source, dest, offset, 0, WHITE);

    //Bubbles
    source = (Rectangle){roundf(animTime * 100), 0, 320, 128};
    dest = (Rectangle){0, 52 * currentWindowScale, GetScreenWidth(), 128 * currentWindowScale};
    DrawTexturePro(rm.bubbleFg, source, dest, offset, 0, WHITE);

    //Floor
    source = (Rectangle){roundf(animTime * 110), 0, 320, 32};
    dest = (Rectangle){0, 148 * currentWindowScale, GetScreenWidth(), 32 * currentWindowScale};
    DrawTexturePro(rm.floorFg, source, dest, offset, 0, WHITE);
}

/**
 * @brief Toggles HUD visibility.
 */
void toggleHUD(void) {
    hudVisible = !hudVisible;
}

/**
 * @brief Toggles invulnerability.
 */
void toggleOxygen(void) {
    pauseOxygen = !pauseOxygen;
}

/**
 * @brief Toggles random speed for NPCs.
 */
void toggleRandomSpeed(void) {
    randomSpeed = !randomSpeed;
}

/**
 * @brief Toggles fish visuals for the player.
 */
void toggleFishPlayer(void) {
    drawPlayerAsFish = !drawPlayerAsFish;
}

/**
 * @brief Toggles animal spawning.
 */
void toggleAnimals(void) {
    onlySpawnGarbage = !onlySpawnGarbage;
}

/**
 * @brief Sets the current NPC spawn interval.
 */
void setInterval(GameWorld *gw, float t) {
    gw->spawnInterval = t;
}