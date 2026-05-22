#include <stdlib.h>
#include <stdbool.h>

#include "raylib/raylib.h"

#include "GameWindow.h"
#include "ResourceManager.h"
#include "GlobalVariables.h"
#include "GameWorld.h"
#include "Menu.h"

const int globalPixelWidth = 320;
const int globalPixelHeight = 180;
const int globalWaterSurfaceHeight = 60;
const int globalFloorHeight = 172;
int currentWindowScale = 2;
int score = 0;
int hiscore = 0;

bool debug = false;

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
    bool initAudio ) {

    GameWindow *gw = (GameWindow*) malloc(sizeof(GameWindow));
    gw->width = width;
    gw->height = height;
    gw->title = title;
    gw->targetFPS = targetFPS;
    gw->antialiasing = antialiasing;
    gw->resizable = resizable;
    gw->fullScreen = fullScreen;
    gw->undecorated = undecorated;
    gw->alwaysOnTop = alwaysOnTop;
    gw->invisibleBackground = invisibleBackground;
    gw->alwaysRun = alwaysRun;
    gw->loadResources = loadResources;
    gw->initAudio = initAudio;
    gw->initialized = false;
    gw->gw = NULL;
    return gw;
}

void initGameWindow(GameWindow *gameWindow) {
    if(!gameWindow->initialized) {
        gameWindow->initialized = true;
    
        //Window properties
        if(gameWindow->antialiasing) {
            SetConfigFlags(FLAG_MSAA_4X_HINT);
        }

        if( gameWindow->resizable) {
            SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        }

        if(gameWindow->fullScreen) {
            SetConfigFlags(FLAG_FULLSCREEN_MODE);
        }

        if(gameWindow->undecorated) {
            SetConfigFlags(FLAG_WINDOW_UNDECORATED);
        }

        if(gameWindow->alwaysOnTop) {
            SetConfigFlags(FLAG_WINDOW_TOPMOST);
        }

        if(gameWindow->invisibleBackground) {
            SetConfigFlags(FLAG_WINDOW_TRANSPARENT);
        }

        if(gameWindow->alwaysRun) {
            SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
        }

        if(gameWindow->initAudio) {
            InitAudioDevice();
        }

        SetTargetFPS(gameWindow->targetFPS);

        InitWindow(gameWindow->width, gameWindow->height, gameWindow->title);
        gameWindow->gw = createGameWorld(GAME_MENU);

        if(gameWindow->loadResources) {
            loadResourcesResourceManager();
        }
    
        SetWindowIcon(rm.icon);
        PlayMusicStream(rm.bg_tune);
        SetMusicVolume(rm.bg_tune, 1.0f);

        //Main game loop
        while (!WindowShouldClose()) {

            UpdateMusicStream(rm.bg_tune);

            switch (gameWindow->gw->gameState) 
            {
                case GAME_MENU:                 
                    SetMusicVolume(rm.bg_tune, 0.2f);
                    SetMusicPitch(rm.bg_tune, 1.0f);
                    drawMainMenu(&gameWindow->gw->gameState);
                    break;

                case GAME_CREDITS:
                    drawMenuCredits(&gameWindow->gw->gameState);
                    break;

                case GAME_RUNNING:
                    SetMusicVolume(rm.bg_tune, 0.5f); // Normal volume during gameplay
                    SetMusicPitch(rm.bg_tune, 1.0f);                    
                    updateGameWorld(gameWindow->gw, GetFrameTime());
                    drawGameWorld(gameWindow->gw);

                    break;

                case GAME_MENU_CONTROLS:
                    drawMenuControls(&gameWindow->gw->gameState);
                    break;

                case GAME_PAUSE_CONTROLS:
                    drawMenuControls(&gameWindow->gw->gameState);
                    break;

                case GAME_OVER:
                    drawMenuGameOver(&gameWindow->gw->gameState);
                    break;

                case GAME_PAUSED:
                    SetMusicVolume(rm.bg_tune, 0.2f); // Muffled volume
                    SetMusicPitch(rm.bg_tune, 0.5f); // Slightly lower pitch
                    drawMenuPause(&gameWindow->gw->gameState);
                    break;

                case GAME_MENU_RESET:
                    gameWindow->gw = createGameWorld(GAME_MENU);
                    drawMainMenu(&gameWindow->gw->gameState);
                    break;

                case GAME_RUNNING_RESET:
                    gameWindow->gw = createGameWorld(GAME_RUNNING);
                    drawGameWorld(gameWindow->gw);
                    updateGameWorld(gameWindow->gw, GetFrameTime());
                    break;

                default:
                    break;
            }

            //Toggle window size
            if(IsKeyPressed(KEY_F4)) {
                int scaleFactor = 2;
                int newWidth = GetScreenWidth() + globalPixelWidth * scaleFactor;
                int newHeight = GetScreenHeight() + globalPixelHeight * scaleFactor;

                if(GetMonitorWidth(0) >= newWidth && GetMonitorHeight(0) >= newHeight) {
                    SetWindowSize(newWidth, newHeight);
                    currentWindowScale += scaleFactor;

                    //If the new resolution is the same as the monitor
                    if(GetMonitorWidth(0) == newWidth && GetMonitorHeight(0) == newHeight) {
                        ToggleBorderlessWindowed();
                    }
                }
                else {
                    ToggleBorderlessWindowed();
                    currentWindowScale = scaleFactor;
                    SetWindowSize(globalPixelWidth * currentWindowScale, globalPixelHeight * currentWindowScale);
                }

                //Center the window
                SetWindowPosition((GetMonitorWidth(0) - GetScreenWidth()) / 2, (GetMonitorHeight(0) - GetScreenHeight()) / 2);
            }

            //Debug commands
            if(debug) {
                if(IsKeyPressed(KEY_F1)) toggleHUD();
                if(IsKeyPressed(KEY_F2)) toggleOxygen();
                if(IsKeyPressed(KEY_F3)) toggleRandomSpeed();
                if(IsKeyPressed(KEY_F5)) {
                    toggleFishPlayer();
                    toggleAnimals();
                }
                if(IsKeyPressed(KEY_ZERO)) setInterval(gameWindow->gw, __FLT_MAX__);
                if(IsKeyPressed(KEY_ONE)) setInterval(gameWindow->gw, INITIAL_SPAWN_INTERVAL);
                if(IsKeyPressed(KEY_TWO)) setInterval(gameWindow->gw, 0.5);
                if(IsKeyPressed(KEY_THREE)) setInterval(gameWindow->gw, 0.005);
            }
        }

        if(gameWindow->loadResources) {
            unloadResourcesResourceManager();
        }

        destroyGameWorld(gameWindow->gw);

        if(gameWindow->initAudio) {
            CloseAudioDevice();
        }

        CloseWindow();
    }


}

