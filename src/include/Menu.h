#ifndef MENU_H
#define MENU_H

#include "enums.h"

//desenha telas do menu e define funcoes dos botoes
void drawMainMenu(State *gameState);
void drawMenuCredits(State *gameState);
void drawMenuControls(State *gameState);
void drawMenuGameOver(State *gameState);
void drawMenuPause(State *gameState);

#endif