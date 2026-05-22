#ifndef GAMEMECHANICS_H
#define GAMEMECHANICS_H

#include "GameWorld.h"
#include "Player.h"
#include "Npc.h"

void checkNpcCollision(Player* p, Npc* n);

void checkNpcCapture(GameWorld *gw, Player* p, Npc* n);

void awardCollisionBonus(Player* p, Npc* n);

void awardCaptureBonus(Player* p, Npc* n);

#endif