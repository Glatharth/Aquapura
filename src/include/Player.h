#ifndef PLAYER_H
#define PLAYER_H

#include "raylib/raylib.h"
#include "enums.h"

#define MAX_OXYGEN 100

typedef struct Player {
    Rectangle collision;
    float oxygen;
    float damageCooldown;
    Vector2 speed;
    float netTimer;
    int netOffset;
    Vector2 netSize;
    Direction lastDir;
} Player;

Player* createPlayer(void);

void updatePlayer(Player *p, float delta);

void drawPlayer(Player *p, float timer);

void drawOxygenBar(Player *p);

#endif