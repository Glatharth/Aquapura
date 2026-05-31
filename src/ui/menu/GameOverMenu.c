#include "raylib/raylib.h"

#include "UI.h"
#include "GameWindow.h"
#include "GlobalVariables.h"
#include "ResourceManager.h"
#include "Enums.h"
#include "Score.h"

Menu *createGameOverMenu(void) {
    Menu *m = buildMenu(2);
    m->background = &rm.menuGameOver;

    m->escapeAction = setGameState;

    Rectangle btnMenu = { 42, 121, 72, 25 };
    Rectangle btnAgain  = { 204, 121, 72, 25 };

    m->buttons[0] = createButton(&rm.menuButton, btnMenu, setGameState, GAME_MENU);
    m->buttons[1] = createButton(&rm.againButton, btnAgain, setGameState, GAME_RUNNING);
    m->selectedButton = 1;

    return m;
}

void drawGameOverMenu(void *menu, float alpha, void *additionalData) {
    drawGenericMenu(menu, alpha, additionalData);
    drawBestScore(true);
}