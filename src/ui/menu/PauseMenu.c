#include <stddef.h>
#include "raylib/raylib.h"

#include "UI.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "Player.h"
#include "ResourceManager.h"
#include "Enums.h"

Menu *createPauseMenu(void) {
    Menu *m = buildMenu(3);
    m->background = &rm.menuPause;

    m->escapeAction = setGameState;
    m->targetState = GAME_RUNNING;

    Rectangle btnControles = { 42, 121, 72, 25 };
    Rectangle btnVoltar = { 123, 121, 72, 25 };
    Rectangle btnMenu  = { 204, 121, 72, 25 };

    m->buttons[0] = createButton(&rm.controlsButton, btnControles, setGameState, GAME_CONTROLS);
    m->buttons[1] = createButton(&rm.backButton2, btnVoltar, setGameState, GAME_RUNNING);
    m->buttons[2] = createButton(&rm.menuButton, btnMenu, setGameState, GAME_MENU);
    m->focusedButton = 1;

    return m;
}

void drawPauseMenu(void *menu, float alpha, void *additionalData) {
    GameWorld *gw = (GameWorld*)((GameWindow*)additionalData)->gameWorld;

    if(gw != NULL) {
        float pausedAlpha = (gw->gameTime - gw->previousTime) * INGAME_TICK_RATE;
        drawGameWorld(gw, pausedAlpha, additionalData);
    }

    drawGenericMenu(menu, alpha, additionalData);
}