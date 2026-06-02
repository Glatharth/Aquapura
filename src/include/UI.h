#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "raylib/raylib.h"
#include "Enums.h"

typedef struct Button {
    Texture2D *texture;
    Rectangle bounds;
    bool focused;
    bool hovered;
    void (*action)(void*, State);           //Params: game window, state
    State targetState;
} Button;

typedef struct Menu {
    Texture2D *background;
    void (*escapeAction)(void*, State);     //Params: game window, state
    State targetState;
    int mainButton;
    int focusedButton;
    int buttonCount;
    Button *buttons[];
} Menu;

Menu *buildMenu(int buttonCount);
void destroyMenu(Menu **m);

Button *createButton(Texture2D *texture, Rectangle bounds, void (*action)(void*, State), State targetState);
void updateFocusedButton(Menu *m);
void resetFocusedButton(Menu *m);

void updateGenericMenu(void *menu, float delta, void* additionalData);
void drawGenericMenu(void *menu, float alpha, void* additionalData);

Menu *createMainMenu(void);
void drawMainMenu(void *menu, float alpha, void* additionalData);

Menu *createPauseMenu(void);
void drawPauseMenu(void *menu, float alpha, void* additionalData);

Menu *createGameOverMenu(void);
void drawGameOverMenu(void *menu, float alpha, void* additionalData);

Menu *createCreditsMenu(void);

Menu *createControlsMenu(void);

//Drawing utilities
void drawButton(Button *b);
void drawMenuBackground(Menu* m);
void drawMenuButtons(Menu* m);

#endif