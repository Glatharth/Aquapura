#include "raylib/raylib.h"

#include "UI.h"
#include "GameWindow.h"
#include "ResourceManager.h"
#include "Enums.h"
#include "Score.h"

Menu *createMainMenu(void) {
    Menu *m = buildMenu(3);
    m->background = &rm.menuBg;

    Rectangle btnControles = { 42, 121, 72, 25 };
    Rectangle btnJogar = { 123, 121, 72, 25 };
    Rectangle btnCreditos  = { 204, 121, 72, 25 };

    m->buttons[0] = createButton(&rm.controlsButton, btnControles, setGameState, GAME_CONTROLS);
    m->buttons[1] = createButton(&rm.playButton, btnJogar, setGameState, GAME_RUNNING);
    m->buttons[2] = createButton(&rm.creditsButton, btnCreditos, setGameState, GAME_CREDITS);
    m->selectedButton = 1;

    return m;
}

void drawMainMenu(void *menu, float alpha, void *additionalData) {
    drawGenericMenu(menu, alpha, additionalData);
    drawBestScore(false);
}