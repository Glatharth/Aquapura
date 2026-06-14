#include <stdlib.h>
#include <stdbool.h>

#include "raylib/raylib.h"

#include "GameWindow.h"
#include "ResourceManager.h"
#include "GlobalVariables.h"
#include "GameWorld.h"
#include "UI.h"
#include "Input.h"

extern GameMode currentMode;

int currentWindowScale = 2;

const bool debug = false;

void gameLoopLogic(GameWindow *gw) {
    if(IsMusicStreamPlaying(*gw->music)) UpdateMusicStream(*gw->music);
    handleInputs(gw);

    if(gw->updateTarget != NULL) {
        float desiredDeltaTime = 1.0f / gw->tickRate;

        if (currentMode == MODE_AI_TRAINING) {
            // MODO FAST FORWARD (Acelera 50x a simulação sem sobrecarregar a renderização visual)
            for(int i = 0; i < 50; i++) {
                if(gw->target != NULL) gw->updateTarget(gw->target, desiredDeltaTime, gw);
            }
            drawGameWindow(gw, 1.0f);
        } else {
            static float updateTimer = 0.0f;
            updateTimer += GetFrameTime();
            int logicUpdates = 0;

            while(updateTimer >= desiredDeltaTime && logicUpdates < MAX_UPDATES_PER_FRAME) {
                if(gw->target != NULL) gw->updateTarget(gw->target, desiredDeltaTime, gw);
                updateTimer -= desiredDeltaTime;
                logicUpdates++;
            }
            if(logicUpdates == MAX_UPDATES_PER_FRAME) updateTimer = 0;

            float frameAlpha = updateTimer / desiredDeltaTime;

            drawGameWindow(gw, frameAlpha);
        }
    }
}

void handleInputs(GameWindow *gw) {
    if(IsKeyPressed(KEY_ESCAPE)) {
        sendInputEvent(INPUT_UI_ESCAPE);
        return;
    }

    if(IsKeyPressed(KEY_UP)) sendInputEvent(INPUT_UI_UP);
    if(IsKeyPressed(KEY_LEFT)) sendInputEvent(INPUT_UI_LEFT);
    if(IsKeyPressed(KEY_DOWN)) sendInputEvent(INPUT_UI_DOWN);
    if(IsKeyPressed(KEY_RIGHT)) sendInputEvent(INPUT_UI_RIGHT);
    if(IsKeyPressed(KEY_ENTER)) sendInputEvent(INPUT_UI_SELECT);
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) sendInputEvent(INPUT_UI_CLICK);

    if(IsKeyPressed(KEY_F4)) {
        int scaleFactor = 2;
        int newWidth = GetScreenWidth() + globalPixelWidth * scaleFactor;
        int newHeight = GetScreenHeight() + globalPixelHeight * scaleFactor;

        if(GetMonitorWidth(0) >= newWidth && GetMonitorHeight(0) >= newHeight) {
            SetWindowSize(newWidth, newHeight);
            currentWindowScale += scaleFactor;

            if(GetMonitorWidth(0) == newWidth && GetMonitorHeight(0) == newHeight) {
                ToggleBorderlessWindowed();
            }
        }
        else {
            ToggleBorderlessWindowed();
            currentWindowScale = scaleFactor;
            SetWindowSize(globalPixelWidth * currentWindowScale, globalPixelHeight * currentWindowScale);
        }

        SetWindowPosition((GetMonitorWidth(0) - GetScreenWidth()) / 2, (GetMonitorHeight(0) - GetScreenHeight()) / 2);
    }

    if(gw->state == GAME_RUNNING) {
        // CORREÇÃO: Lê o input do teclado e manda para o PRIMEIRO jogador da rede (Clone 0)
        // Assim você ainda pode testar o teclado caso queira forçar movimentos nele.
        Player *p = gw->gameWorld->jogadores[0];
        
        if (p != NULL) {
            if(IsKeyDown(p->key.moveUp)) sendInputEvent(INPUT_P1_MOVE_UP);
            if(IsKeyDown(p->key.moveLeft)) sendInputEvent(INPUT_P1_MOVE_LEFT);
            if(IsKeyDown(p->key.moveDown)) sendInputEvent(INPUT_P1_MOVE_DOWN);
            if(IsKeyDown(p->key.moveRight)) sendInputEvent(INPUT_P1_MOVE_RIGHT);
            if(IsKeyDown(p->key.capture)) sendInputEvent(INPUT_P1_CAPTURE);
        }

        if(debug) {
            if(IsKeyPressed(KEY_F1)) toggleHUD();
            if(IsKeyPressed(KEY_F2)) toggleOxygen();
            if(IsKeyPressed(KEY_F3)) toggleRandomSpeed();
            if(IsKeyPressed(KEY_F5)) {
                toggleFishPlayer();
                toggleAnimals();
            }
            if(IsKeyPressed(KEY_ZERO)) setInterval(gw->gameWorld, __FLT_MAX__);
            if(IsKeyPressed(KEY_ONE)) setInterval(gw->gameWorld, INITIAL_SPAWN_INTERVAL);
            if(IsKeyPressed(KEY_TWO)) setInterval(gw->gameWorld, 0.5);
            if(IsKeyPressed(KEY_THREE)) setInterval(gw->gameWorld, 0.005);
        }
    }
}

GameWindow* createGameWindow(
    int width, int height, const char *title, int targetFPS,
    bool antialiasing, bool resizable, bool fullScreen,
    bool undecorated, bool alwaysOnTop, bool invisibleBackground, 
    bool alwaysRun, bool loadResources, bool initAudio ) {

    GameWindow *gw = malloc(sizeof(*gw));
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
    gw->tickRate = MENU_TICK_RATE;

    gw->gameWorld = NULL;
    gw->mainMenu = NULL;
    gw->controlsMenu = NULL;
    gw->creditsMenu = NULL;
    gw->gameOverMenu = NULL;
    gw->pauseMenu = NULL;

    gw->music = NULL;
    gw->updateTarget = NULL;
    gw->drawTarget = NULL;
    gw->updateTarget = NULL;

    return gw;
}

void initGameWindow(GameWindow *gw) {
    if(!gw->initialized) {
        gw->initialized = true;
    
        if(gw->targetFPS == 0) SetConfigFlags(FLAG_VSYNC_HINT);
        if(gw->antialiasing) SetConfigFlags(FLAG_MSAA_4X_HINT);
        if(gw->resizable) SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        if(gw->fullScreen) SetConfigFlags(FLAG_FULLSCREEN_MODE);
        if(gw->undecorated) SetConfigFlags(FLAG_WINDOW_UNDECORATED);
        if(gw->alwaysOnTop) SetConfigFlags(FLAG_WINDOW_TOPMOST);
        if(gw->invisibleBackground) SetConfigFlags(FLAG_WINDOW_TRANSPARENT);
        if(gw->alwaysRun) SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
        if(gw->initAudio) InitAudioDevice();

        SetTargetFPS(gw->targetFPS);
        InitWindow(gw->width, gw->height, gw->title);
        SetExitKey(KEY_NULL);

        if(gw->loadResources) loadResourcesResourceManager();

        setGameState(gw, GAME_MENU);

        while(!WindowShouldClose()) {
            gameLoopLogic(gw);
        }

        if(gw->loadResources) {
            unloadResourcesResourceManager();
        }

        if(gw->initAudio) {
            CloseAudioDevice();
        }

        destroyGameWindow(&gw);
        CloseWindow();
    }
}

void setGameState(void *gameWindow, State newState) {
    GameWindow *gw = (GameWindow*)gameWindow;

    switch(newState) {
        case GAME_MENU:
            SetExitKey(KEY_ESCAPE);
            destroyGameWorld(&gw->gameWorld);
            if(gw->mainMenu == NULL) gw->mainMenu = createMainMenu();
            gw->tickRate = MENU_TICK_RATE;
            gw->updateTarget = updateGenericMenu;
            gw->drawTarget = drawMainMenu;
            gw->target = gw->mainMenu;
            gw->music = &rm.bg_tune;
            SetMusicVolume(*gw->music, 0.2f);
            SetMusicPitch(*gw->music, 1.0f);
            break;

        case GAME_RUNNING:
            SetExitKey(KEY_NULL);
            if(gw->gameWorld == NULL) {
                gw->gameWorld = createGameWorld();
                score = 0;
            }
            gw->tickRate = INGAME_TICK_RATE;
            gw->updateTarget = updateGameWorld;
            gw->drawTarget = drawGameWorld;
            gw->target = gw->gameWorld;
            gw->music = &rm.bg_tune;
            SetMusicVolume(*gw->music, 0.5f); 
            SetMusicPitch(*gw->music, 1.0f);

            resetFocusedButton(gw->mainMenu);
            resetFocusedButton(gw->controlsMenu);
            resetFocusedButton(gw->creditsMenu);
            resetFocusedButton(gw->gameOverMenu);
            resetFocusedButton(gw->pauseMenu);
            break;

        case GAME_OVER:
            SetExitKey(KEY_NULL);
            destroyGameWorld(&gw->gameWorld);
            if(gw->gameOverMenu == NULL) gw->gameOverMenu = createGameOverMenu();
            gw->tickRate = MENU_TICK_RATE;
            gw->updateTarget = updateGenericMenu;
            gw->drawTarget = drawGameOverMenu;
            gw->target = gw->gameOverMenu;
            gw->music = &rm.bg_tune;
            break;

        case GAME_PAUSED:
            SetExitKey(KEY_NULL);
            if(gw->pauseMenu == NULL) gw->pauseMenu = createPauseMenu();
            gw->tickRate = MENU_TICK_RATE;
            gw->updateTarget = updateGenericMenu;
            gw->drawTarget = drawPauseMenu;
            gw->target = gw->pauseMenu;
            gw->music = &rm.bg_tune;
            SetMusicVolume(*gw->music, 0.2f); 
            SetMusicPitch(*gw->music, 0.5f); 
            break;

        case GAME_CONTROLS:
            SetExitKey(KEY_NULL);
            if(gw->controlsMenu == NULL) gw->controlsMenu = createControlsMenu();
            gw->tickRate = MENU_TICK_RATE;
            gw->updateTarget = updateGenericMenu;
            gw->drawTarget = drawGenericMenu;
            gw->target = gw->controlsMenu;
            gw->music = &rm.bg_tune;
            break;

        case GAME_CREDITS:
            SetExitKey(KEY_NULL);
            if(gw->creditsMenu == NULL) gw->creditsMenu = createCreditsMenu();
            gw->tickRate = MENU_TICK_RATE;
            gw->updateTarget = updateGenericMenu;
            gw->drawTarget = drawGenericMenu;
            gw->target = gw->creditsMenu;
            gw->music = &rm.bg_tune;
            break;

        default:
            SetExitKey(KEY_ESCAPE);
            gw->tickRate = MENU_TICK_RATE;
            gw->updateTarget = NULL;
            gw->drawTarget = NULL;
            gw->updateTarget = NULL;
            gw->music = NULL;
    }

    if(gw->music != NULL && !IsMusicStreamPlaying(*gw->music)) PlayMusicStream(*gw->music);
    gw->state = newState;
}

void drawGameWindow(GameWindow *gw, float alpha) {
    BeginDrawing();
    if(gw->drawTarget != NULL) gw->drawTarget(gw->target, alpha, gw);
    else ClearBackground(BLACK);
    EndDrawing();
}

void destroyGameWindow(GameWindow **gw) {
    if(gw == NULL || *gw == NULL) return;

    destroyGameWorld(&(*gw)->gameWorld);
    destroyMenu(&(*gw)->mainMenu);
    destroyMenu(&(*gw)->controlsMenu);
    destroyMenu(&(*gw)->creditsMenu);
    destroyMenu(&(*gw)->gameOverMenu);
    destroyMenu(&(*gw)->pauseMenu);

    free(*gw);
    *gw = NULL;
}