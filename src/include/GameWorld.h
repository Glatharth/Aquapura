/**
 * @file GameWorld.h
 * @author Prof. Dr. David Buzatto
 * @brief GameWorld struct and function declarations.
 * 
 * @copyright Copyright (c) 2025
 */
#ifndef GAMEWORLD_H
#define GAMEWORLD_H

#include <stdbool.h>
#include "Player.h"
#include "Npc.h"

//Definitions
#define ENEMY_ESCAPE_LIMIT 5
#define ENEMY_CAUGHT_LIMIT 15
#define MAX_NPC_SPEED 250
#define MAX_NPC 1000
#define INITIAL_SPAWN_INTERVAL 1.0f
#define MIN_SPAWN_INTERVAL 0.25f
#define SPAWN_DECREMENT 0.025f
#define BUBBLE_SPAWN_INTERVAL 6.0f

typedef struct GameWorld {
    Player *player;
    Npc *npc[MAX_NPC];
    int activeNpc;
    float gameTime;
    float previousTime;
    float spawnTimer;
    float spawnInterval;
    float bubbleTimer;
    int escapedEnemies;
    int caughtEnemies;
    int npcSpeed;
} GameWorld;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld* createGameWorld(void);

/**
 * @brief Destroys a GameWorld object and its dependecies.
 */
void destroyGameWorld(GameWorld **gw);

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld(void *gameWorld, float delta, void *additionalData);

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld(void *gameWorld, float alpha, void *additionalData);

/**
 * @brief Draws the environment behind all entities.
 */
void drawBackground(float animTime);

/**
 * @brief Draws the environment in front of all entities.
 */
void drawForeground(float animTime);

/**
 * @brief Toggles HUD visibility.
 */
void toggleHUD(void);

/**
 * @brief Toggles invulnerability.
 */
void toggleOxygen(void);

/**
 * @brief Toggles random speed for NPCs.
 */
void toggleRandomSpeed(void);

/**
 * @brief Toggles fish visuals for the player.
 */
void toggleFishPlayer(void);

/**
 * @brief Toggles animal spawning.
 */
void toggleAnimals(void);

/**
 * @brief Sets the current NPC spawn interval.
 */
void setInterval(GameWorld *gw, float t);

#endif