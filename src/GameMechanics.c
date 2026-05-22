#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"

#include "GameMechanics.h"
#include "GlobalVariables.h"

extern bool pauseOxygen;

void checkNpcCollision(Player* p, Npc* n) {
    //Only check for collision if the player is not intangible
    if(p->damageCooldown == 0) {
        //Checks if the NPC collided with the player
        if(CheckCollisionRecs(p->collision, n->collision)){
            awardCollisionBonus(p, n);

            //If the NPC is set to be removed after a collision, mark it for removal
            if(n->removeOnCollision) {
                n->shouldBeRemoved = true;
            }
        }
    }
}

void checkNpcCapture(GameWorld *gw, Player* p, Npc* n) {
    //Checks if the player is currently using the net
    if(p->netTimer > 0 && p->netTimer < 0.25) {
        Rectangle netRec = {
            p->collision.x + (p->collision.width - p->netSize.x) / 2,
            p->collision.y + (p->collision.height - p->netSize.y) / 2,
            p->netSize.x,
            p->netSize.y
        };

        switch(p->lastDir) {
            case LEFT: netRec.x -= p->netOffset; break;
            default: netRec.x += p->netOffset;
        }

        if(CheckCollisionRecs(netRec, n->collision)) {
            //Mark NPC for removal
            n->shouldBeRemoved = true;

            awardCaptureBonus(p, n);

            if(n->type == NPC_GARBAGE) {
                gw->caughtEnemies++;
            }
        }
    }
}

void awardCollisionBonus(Player* p, Npc* n) {
    //Awards the player with the NPC's oxygen bonus. May be positive or negative.
    if(!pauseOxygen) p->oxygen = fmin(p->oxygen + n->collisionOxygen, MAX_OXYGEN);

    //If the oxygen gain is negative, damage the player
    if(n->collisionOxygen < 0) {
        p->damageCooldown = 1;
    }
}

void awardCaptureBonus(Player* p, Npc* n) {
    //Awards the player with the NPC's score and oxygen bonuses. May be positive or negative.
    score += n->captureScore;
    p->oxygen = fmin(p->oxygen + n->captureOxygen, MAX_OXYGEN);

    //If the oxygen gain is negative, damage the player
    if(n->captureOxygen < 0) {
        p->damageCooldown = 1;
    }
}