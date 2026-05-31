#ifndef PLAYER_H
#define PLAYER_H

#include "raylib/raylib.h"
#include "Enums.h"

#define MAX_OXYGEN 100

typedef struct PlayerKeybind {
    KeyboardKey moveUp;
    KeyboardKey moveLeft;
    KeyboardKey moveDown;
    KeyboardKey moveRight;
    KeyboardKey capture;
} PlayerKeybind;

typedef struct Player {
    Rectangle collision;
    Vector2 realPos;
    Vector2 prevPos;
    float oxygen;
    float damageCooldown;
    Vector2 speed;
    float netTimer;
    int netOffset;
    Vector2 netSize;
    Direction lastDir;
    PlayerKeybind key;
} Player;

Player* createPlayer(void);

void updatePlayer(Player *p, float delta);

void drawPlayer(Player *p, float alpha, float animTime);

void drawOxygenBar(Player *p);

#endif