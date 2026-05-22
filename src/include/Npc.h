#ifndef NPC_H
#define NPC_H

#include <stdbool.h>
#include "raylib/raylib.h"
#include "enums.h"

typedef struct Npc {
    Rectangle collision;
    NPCType type;
    Vector2 speed;
    int collisionOxygen;
    int captureOxygen;
    int captureScore;
    int variant;
    bool removeOnCollision;
    bool shouldBeRemoved;
    float removalCountdown;
}Npc;

Npc* createNpc(float speed);

Npc* createBubble(float speed);

void drawNpc(Npc* n);

void drawBubble(Npc* n);

void updateNpc(Npc *n, float delta);

#endif