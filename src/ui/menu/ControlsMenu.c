#include <stddef.h>
#include "raylib/raylib.h"

#include "UI.h"
#include "GameWindow.h"
#include "ResourceManager.h"
#include "Enums.h"

static void setStateIfWorldIsNull(void* gameWindow, State newState) {
    GameWindow *gw = (GameWindow*)gameWindow;

    if(gw != NULL) {
        if(gw->gameWorld == NULL) setGameState(gameWindow, newState);
        else setGameState(gameWindow, GAME_PAUSED);
    }
}

Menu *createControlsMenu(void) {
    Menu *m = buildMenu(1);
    m->background = &rm.menuControls;

    m->escapeAction = setStateIfWorldIsNull;

    Rectangle btnVoltar = {9, 7, 23, 23};

    m->buttons[0] = createButton(&rm.backButton, btnVoltar, setStateIfWorldIsNull, GAME_MENU);

    return m;
} 