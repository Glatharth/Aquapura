#ifndef NPC_H
#define NPC_H

#include <stdbool.h>
#include "raylib/raylib.h"
#include "Enums.h"

typedef struct Npc {
    Rectangle collision;
    Vector2 realPos;
    Vector2 prevPos;
    NPCType type;
    Vector2 speed;
    float spawnLocation;
    int collisionOxygen;
    int captureOxygen;
    int captureScore;
    int variant;
    bool removeOnCollision;
    bool shouldBeRemoved;
    float waveTime;
    int waveAmplitude;
    float removalCountdown;
} Npc;

Npc* createNpc(float speed);

Npc* createBubble(float speed);

void drawNpc(Npc* n, float alpha);

void drawBubble(Npc* n, float alpha);

void updateNpc(Npc *n, float delta);

#endif