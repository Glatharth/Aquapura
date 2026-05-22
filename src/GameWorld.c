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
#include "GlobalVariables.h"
#include "ResourceManager.h"
#include "utils.h"

#include "Player.h"
#include "Npc.h"
#include "GameMechanics.h"
#include "Score.h"

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
GameWorld* createGameWorld( State initialState ) { //initialize the gameworld with the initial values
    score = 0;

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    gw->player = createPlayer();
    for(int i = 0; i < MAX_NPC; i++){
        gw->npc[i] = NULL;
    }
    gw->activeNpc = 0;
    gw->timer = 0.0f;
    gw->spawnTimer = 0.0f;
    gw->spawnInterval = INITIAL_SPAWN_INTERVAL;
    gw->bubbleTimer = 0;
    gw->gameState = initialState;
    gw->escapedEnemies = 0;
    gw->caughtEnemies = 0;
    gw->npcSpeed = 60;
    return gw;
}

/**
 * @brief Destroys a GameWindow object and its dependecies.
 */
void destroyGameWorld( GameWorld *gw ) { //free the gameworld from the memory
    for (int i = 0; i < MAX_NPC; i++) {
        if (gw->npc[i] != NULL) {
            free(gw->npc[i]);
        }
    }
    free( gw->player ); 
    free( gw );
}

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld( GameWorld *gw, float delta ) { //update the gameworld with all its components
    gw->timer += delta;
    gw->spawnTimer += delta;
    gw->bubbleTimer += delta;

    updatePlayer( gw->player, delta );

    //Pause function
    if(gw->gameState == GAME_RUNNING){
        if(IsKeyPressed(KEY_P)){
            gw->gameState = GAME_PAUSED;
        }
    }

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
            gw->gameState = GAME_OVER;
        }
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
}


/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) { //draws the gameworld with all its components

    BeginDrawing();

    drawBackground(gw->timer);

    drawPlayer(gw->player, gw->timer);

    for (int i = 0; i < MAX_NPC; i++) {
        if(gw->npc[i] != NULL) {
            if(gw->npc[i]->type == NPC_BUBBLE) {
                drawBubble(gw->npc[i]);
            }
            else {
                drawNpc(gw->npc[i]);
            }
        }
    }

    drawForeground(gw->timer);
    
    //Water surface height
    //DrawLine(0, globalWaterSurfaceHeight * currentWindowScale, GetScreenWidth(), globalWaterSurfaceHeight * currentWindowScale, BLUE);

    if(hudVisible) {
        drawOxygenBar(gw->player);
        drawInGameScore();
    }

    EndDrawing();

}

/**
 * @brief Draws the environment behind all entities.
 */
void drawBackground( float time ) {
    Texture2D *celestial;
    Color skyColor;
    Color cloudShadowColor;
    Color cloudHighlightColor;
    Color cityscapeColor;
    Color cityOverlayColor;
    Color waterColor;

    int dayTimeInterval = 60;

    //Used for updating the color of certain background elements
    float colorLerpProgress = fmin(fmax(0, fmod(time, dayTimeInterval) - dayTimeInterval + 1), 1);

    switch((int)(time / dayTimeInterval) % 4) {
        case 1: //Sunset
            celestial = &rm.sunBg;
            skyColor = interpolateColor((Color){255, 108, 36, 255}, (Color){29, 43, 83, 255}, colorLerpProgress);
            cloudHighlightColor = interpolateColor((Color){255, 241, 232, 255}, (Color){194, 195, 199, 255}, colorLerpProgress);
            cloudShadowColor = interpolateColor((Color){255, 204, 170, 255}, (Color){131, 118, 156, 255}, colorLerpProgress);
            cityscapeColor = interpolateColor((Color){73, 51, 59, 255}, (Color){0, 0, 0, 255}, colorLerpProgress);
            cityOverlayColor = interpolateColor((Color){73, 51, 59, 255}, (Color){243, 239, 125, 255}, colorLerpProgress);
            waterColor = interpolateColor((Color){18, 83, 89, 255}, (Color){17, 29, 53, 255}, colorLerpProgress);
            break;

        case 2: //Night
            celestial = &rm.moonBg;
            skyColor = interpolateColor((Color){29, 43, 83, 255}, (Color){117, 70, 101, 255}, colorLerpProgress);
            cloudHighlightColor = interpolateColor((Color){194, 195, 199, 255}, (Color){255, 157, 129, 255}, colorLerpProgress);
            cloudShadowColor = interpolateColor((Color){131, 118, 156, 255}, (Color){255, 110, 89, 255}, colorLerpProgress);
            cityscapeColor = interpolateColor((Color){0, 0, 0, 255}, (Color){74, 51, 59, 255}, colorLerpProgress);
            cityOverlayColor = (Color){243, 239, 125, 255};
            waterColor = interpolateColor((Color){17, 29, 53, 255}, (Color){18, 83, 89, 255}, colorLerpProgress);
            break;

        case 3: //Sunrise
            celestial = &rm.moonBg;
            skyColor = interpolateColor((Color){117, 70, 101, 255}, (Color){41, 173, 255, 255}, colorLerpProgress);
            cloudHighlightColor = interpolateColor((Color){255, 157, 129, 255}, (Color){255, 241, 232, 255}, colorLerpProgress);
            cloudShadowColor = interpolateColor((Color){255, 110, 89, 255}, (Color){194, 195, 199, 255}, colorLerpProgress);
            cityscapeColor = interpolateColor((Color){74, 51, 59, 255}, (Color){6, 90, 181, 255}, colorLerpProgress);
            cityOverlayColor = interpolateColor((Color){243, 239, 125, 255}, (Color){6, 90, 181, 255}, colorLerpProgress);
            waterColor = (Color){18, 83, 89, 255};
            break;

        default:
            celestial = &rm.sunBg;
            skyColor = interpolateColor((Color){41, 173, 255, 255}, (Color){255, 108, 36, 255}, colorLerpProgress);
            cloudHighlightColor = (Color){255, 241, 232, 255};
            cloudShadowColor = interpolateColor((Color){194, 195, 199, 255}, (Color){255, 204, 170, 255}, colorLerpProgress);
            cityscapeColor = interpolateColor((Color){6, 90, 181, 255}, (Color){73, 51, 59, 255}, colorLerpProgress);
            cityOverlayColor = interpolateColor((Color){6, 90, 181, 255}, (Color){73, 51, 59, 255}, colorLerpProgress);
            waterColor = (Color){18, 83, 89, 255};
    }

    Rectangle source;
    Rectangle dest;
    Vector2 offset = {0, 0};

    //Sky
    DrawRectangle(0, 0, GetScreenWidth(), 64 * currentWindowScale, skyColor);

    //Celestial
    float celestialAngle = (fmod((time + 1) / dayTimeInterval / 2, 1)) * PI;
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
    source = (Rectangle){(int)time, 0, 320, 180};
    dest = (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()};
    DrawTexturePro(rm.cityscapeBg, source, dest, offset, 0, cityscapeColor);
    DrawTexturePro(rm.cityOverlayBg, source, dest, offset, 0, cityOverlayColor);

    //Water
    dest = (Rectangle){0, 52 * currentWindowScale, GetScreenWidth(), 128 * currentWindowScale};
    source = (Rectangle){320 + (int)(time * 70), 0, 320, 128};
    DrawTexturePro(rm.waterBg, source, dest, offset, 0, waterColor);
    source.height /= 4;
    dest.height /= 4;
    DrawTexturePro(rm.foamFg, source, dest, offset, 0, WHITE);
    source = (Rectangle){(int)(time * 90), 0, 320, 128};
    dest.height *= 4;
    DrawTexturePro(rm.waterBg, source, dest, offset, 0, waterColor);

    //Floor
    source = (Rectangle){(int)(time * 40), 0, 320, 64};
    dest = (Rectangle){0, 116 * currentWindowScale, GetScreenWidth(), 64 * currentWindowScale};
    DrawTexturePro(rm.floorBg, source, dest, offset, 0, WHITE);
}

/**
 * @brief Draws the environment in front of all entities.
 */
void drawForeground( float time ) {
    Rectangle source;
    Rectangle dest;
    Vector2 offset = {0, 0};

    //Foam
    source = (Rectangle){(int)(time * 90), 0, 320, 32};
    dest = (Rectangle){0, 52 * currentWindowScale, GetScreenWidth(), 32 * currentWindowScale};
    DrawTexturePro(rm.foamFg, source, dest, offset, 0, WHITE);

    //Bubbles
    source = (Rectangle){(int)(time * 100), 0, 320, 128};
    dest = (Rectangle){0, 52 * currentWindowScale, GetScreenWidth(), 128 * currentWindowScale};
    DrawTexturePro(rm.bubbleFg, source, dest, offset, 0, WHITE);

    //Floor
    source = (Rectangle){(int)(time * 110), 0, 320, 32};
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