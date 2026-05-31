#include "raylib/raylib.h"

#include "UI.h"
#include "GameWindow.h"
#include "ResourceManager.h"
#include "Enums.h"

Menu *createCreditsMenu(void) {
    Menu *m = buildMenu(1);
    m->background = &rm.menuCredits;

    m->escapeAction = setGameState;

    Rectangle btnVoltar = {9, 7, 23, 23};

    m->buttons[0] = createButton(&rm.backButton, btnVoltar, setGameState, GAME_MENU);

    return m;
}