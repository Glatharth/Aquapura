/**
 * @file GameWorld.h
 * @author Prof. Dr. David Buzatto
 * @brief GameWorld struct and function declarations.
 * 
 * @copyright Copyright (c) 2025
 */
#ifndef GAMEWORLD_H
#define GAMEWORLD_H

#include "Player.h"
#include "Npc.h"
#include "enums.h"

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
    float timer;
    float spawnTimer;
    float spawnInterval;
    float bubbleTimer;
    State gameState;
    int escapedEnemies;
    int caughtEnemies;
    int npcSpeed;
} GameWorld;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld* createGameWorld( State initialState );

/**
 * @brief Destroys a GameWindow object and its dependecies.
 */
void destroyGameWorld( GameWorld *gw );

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld( GameWorld *gw, float delta );

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw );

/**
 * @brief Draws the environment behind all entities.
 */
void drawBackground( float time );

/**
 * @brief Draws the environment in front of all entities.
 */
void drawForeground( float time );

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