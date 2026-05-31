/**
 * @file GameWindow.h
 * @author Prof. Dr. David Buzatto
 * @brief GameWindow struct and function declarations.
 * 
 * @copyright Copyright (c) 2025
 */
#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <stdbool.h>
#include "Enums.h"

#define MAX_UPDATES_PER_FRAME 5
#define INGAME_TICK_RATE 60
#define MENU_TICK_RATE 20

typedef struct GameWindow {

    int width;
    int height;
    const char *title;

    int targetFPS;
    bool antialiasing;
    bool resizable;
    bool fullScreen;
    bool undecorated;
    bool alwaysOnTop;
    bool invisibleBackground;
    bool alwaysRun;
    bool loadResources;
    bool initAudio;

    State state;
    struct GameWorld *gameWorld;
    struct Menu *mainMenu;
    struct Menu *controlsMenu;
    struct Menu *creditsMenu;
    struct Menu *gameOverMenu;
    struct Menu *pauseMenu;
    int tickRate;

    struct Music *music;
    void (*updateTarget)(void*, float, void*);    //Params: update target, delta time, additional data
    void (*drawTarget)(void*, float, void*);      //Params: update target, frame alpha, additional data
    void *target;

    bool initialized;
} GameWindow;

/**
 * @brief Creates a dinamically allocated GameWindow struct instance.
 */
GameWindow* createGameWindow(
        int width, 
        int height, 
        const char *title, 
        int targetFPS,
        bool antialiasing, 
        bool resizable, 
        bool fullScreen,
        bool undecorated, 
        bool alwaysOnTop, 
        bool invisibleBackground, 
        bool alwaysRun, 
        bool loadResources, 
        bool initAudio );

/**
 * @brief Initializes the Window, starts the game loop and, when it
 * finishes, the window will be finished and destroyed too.
 */
void initGameWindow(GameWindow *gw);

/**
 * @brief Destroys a GameWindow object and its dependecies.
 */
void destroyGameWindow(GameWindow **gw);

/**
 * @brief Handles the logic behind updating and drawing new frames.
 */
void gameLoopLogic(GameWindow *gw);

/**
 * @brief Handles the logic behind immediate key presses.
 */
void handleInputs(GameWindow *gw);

/**
 * @brief Changes the current game state and does all the necessary work behind it.
 */
void setGameState(void *gameWindow, State newState);

/**
 * @brief Processes the current drawing function of the game window.
 */
void drawGameWindow(GameWindow *gw, float alpha);

#endif