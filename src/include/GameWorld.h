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

#define MAX_PLAYERS 100 // Limite máximo de jogadores simultâneos para a IA

typedef struct GameWorld {
    Player *jogadores[MAX_PLAYERS]; 
    double pontuacoes[MAX_PLAYERS]; 
    int numJogadoresAtivos;         
    int maxJogadoresAtuais;         
    
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

GameWorld* createGameWorld(void);
void destroyGameWorld(GameWorld **gw);
void resetarGameWorld(int qtdPlayers);
void updateGameWorld(void *gameWorld, float delta, void *additionalData);
void drawGameWorld(void *gameWorld, float alpha, void *additionalData);
void drawBackground(float animTime);
void drawForeground(float animTime);
void toggleHUD(void);
void toggleOxygen(void);
void toggleRandomSpeed(void);
void toggleFishPlayer(void);
void toggleAnimals(void);
void setInterval(GameWorld *gw, float t);

float calcularDistanciaInimigoMaisProximo(GameWorld *gw, Player *p);
float calcularDistanciaBolhaMaisProxima(GameWorld *gw, Player *p);

#endif // GAMEWORLD_H