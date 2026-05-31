#ifndef GAMEMECHANICS_H
#define GAMEMECHANICS_H

struct GameWorld;
struct Player;
struct Npc;

void checkNpcCollision(struct Player *p, struct Npc *n);

void checkNpcCapture(struct GameWorld *gw, struct Player *p, struct Npc *n);

void awardCollisionBonus(struct Player *p, struct Npc *n);

void awardCaptureBonus(struct Player *p, struct Npc *n);

#endif