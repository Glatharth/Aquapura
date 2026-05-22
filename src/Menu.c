#include "Menu.h"
#include "GlobalVariables.h"
#include "ResourceManager.h"

#include "GameWorld.h"
#include "Score.h"

#include <raylib/raylib.h>

//desenho e animacao dos botoes
static void DrawButtonAnimation(Rectangle dimensions, Texture2D texture, Vector2 mouse, float scale) {
    Rectangle source = {0, 0, (float)texture.width, (float)texture.height};
    if (CheckCollisionPointRec(mouse, dimensions)) {
        Rectangle scaledDest = {
            (dimensions.x - (dimensions.width * (scale - 1.0f) / 2.0f)),
            (dimensions.y - (dimensions.height * (scale - 1.0f) / 2.0f)),
            dimensions.width * scale,
            dimensions.height * scale
        };
        DrawTexturePro(texture, source, scaledDest, (Vector2){ 0, 0 }, 0.0f, WHITE);
    } else {
        Rectangle dest = {
            dimensions.x,
            dimensions.y,
            dimensions.width,
            dimensions.height
        };
        DrawTexturePro(texture, source, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }
}

void drawMainMenu(State *gameState) 
{
    BeginDrawing();
    ClearBackground(WHITE);

    DrawTexturePro (
        rm.menuBg,
        (Rectangle){0, 0, (float)rm.menuBg.width, (float)rm.menuBg.height},
        (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
     
    //area dos botoes
    Rectangle btnControles = { 42 * currentWindowScale, 121 * currentWindowScale, 72 * currentWindowScale, 25 * currentWindowScale };
    Rectangle btnJogar = { 123 * currentWindowScale, 121 * currentWindowScale, 72 * currentWindowScale, 25 * currentWindowScale };
    Rectangle btnCreditos  = { 204 * currentWindowScale, 121 * currentWindowScale, 72 * currentWindowScale, 25 * currentWindowScale };

    Vector2 mouse = GetMousePosition();
    float scale = 1.1f;

    //animacao dos botoes
    DrawButtonAnimation(btnJogar, rm.playButton, mouse, scale);
    DrawButtonAnimation(btnCreditos, rm.creditsButton, mouse, scale);
    DrawButtonAnimation(btnControles, rm.controlsButton, mouse, scale);

    drawBestScore(false);

    EndDrawing();

    //detecta clique e muda estado do jogo
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if(CheckCollisionPointRec(mouse, btnJogar)) {
            *gameState = GAME_RUNNING;
        }
        else if(CheckCollisionPointRec(mouse, btnCreditos)) {
            *gameState = GAME_CREDITS;
        }
        else if(CheckCollisionPointRec(mouse, btnControles)) {
            *gameState = GAME_MENU_CONTROLS;
        }
    }
}

void drawMenuCredits(State *gameState) 
{
    BeginDrawing();
    ClearBackground(WHITE);

    DrawTexturePro (
        rm.menuCredits,
        (Rectangle){0, 0, (float)rm.menuCredits.width, (float)rm.menuCredits.height},
        (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
     
    Rectangle btnVoltar = {9 * currentWindowScale, 7 * currentWindowScale, 23 * currentWindowScale, 23 * currentWindowScale};
    Vector2 mouse = GetMousePosition();
    float scale = 1.1f;
    DrawButtonAnimation(btnVoltar, rm.backButton, mouse, scale);

    EndDrawing();

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if(CheckCollisionPointRec(mouse, btnVoltar)) {
            *gameState = GAME_MENU;
        }
    }
}

void drawMenuControls(State *gameState) 
{
    BeginDrawing();
    ClearBackground(WHITE);

    DrawTexturePro (
        rm.menuControls,
        (Rectangle){0, 0, (float)rm.menuControls.width, (float)rm.menuControls.height},
        (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
     
    Rectangle btnVoltar = {9 * currentWindowScale, 7 * currentWindowScale, 23 * currentWindowScale, 23 * currentWindowScale};
    Vector2 mouse = GetMousePosition();
    float scale = 1.1f;
    DrawButtonAnimation(btnVoltar, rm.backButton, mouse, scale);

    EndDrawing();

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if(CheckCollisionPointRec(mouse, btnVoltar)) {
            //Ensures the player doesn't go back to the main menu unintentionally
            if(*gameState == GAME_PAUSE_CONTROLS) {
                *gameState = GAME_PAUSED;
            }
            else {
                *gameState = GAME_MENU;
            }
        }
    }
}    

void drawMenuGameOver( State *gameState) {
    BeginDrawing();
    ClearBackground(WHITE);
    
    DrawTexturePro (
        rm.menuGameOver,
        (Rectangle){0, 0, (float)rm.menuGameOver.width, (float)rm.menuGameOver.height},
        (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
        (Vector2){0, 0},
        0.0f,
        WHITE
    );

    Rectangle btnMenu = { 42 * currentWindowScale, 121 * currentWindowScale, 72 * currentWindowScale, 25 * currentWindowScale };
    Rectangle btnAgain  = { 204 * currentWindowScale, 121 * currentWindowScale, 72 * currentWindowScale, 25 * currentWindowScale };

    Vector2 mouse = GetMousePosition();
    float scale = 1.1f;

    DrawButtonAnimation(btnAgain, rm.againButton, mouse, scale);
    DrawButtonAnimation(btnMenu, rm.menuButton, mouse, scale);
    drawBestScore(true);

    EndDrawing();

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if(CheckCollisionPointRec(mouse, btnAgain)) {
            *gameState = GAME_RUNNING_RESET;
        }
        else if(CheckCollisionPointRec(mouse, btnMenu)) {
            *gameState = GAME_MENU_RESET;
        }
    }
}

void drawMenuPause( State *gameState) {
    BeginDrawing();
    
    DrawTexturePro (
        rm.menuPause,
        (Rectangle){0, 0, (float)rm.menuPause.width, (float)rm.menuPause.height},
        (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
        (Vector2){0, 0},
        0.0f,
        WHITE
    );

    Rectangle btnControles = { 42 * currentWindowScale, 121 * currentWindowScale, 72 * currentWindowScale, 25 * currentWindowScale };
    Rectangle btnVoltar = { 123 * currentWindowScale, 121 * currentWindowScale, 72 * currentWindowScale, 25 * currentWindowScale };
    Rectangle btnMenu  = { 204 * currentWindowScale, 121 * currentWindowScale, 72 * currentWindowScale, 25 * currentWindowScale };

    Vector2 mouse = GetMousePosition();
    float scale = 1.1f;

    DrawButtonAnimation(btnMenu, rm.menuButton, mouse, scale);
    DrawButtonAnimation(btnVoltar, rm.backButton2, mouse, scale);
    DrawButtonAnimation(btnControles, rm.controlsButton, mouse, scale);

    EndDrawing();

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if(CheckCollisionPointRec(mouse, btnMenu)) {
            *gameState = GAME_MENU_RESET;
        }
        else if (CheckCollisionPointRec(mouse, btnVoltar)) {
            *gameState = GAME_RUNNING;
        }
        else if(CheckCollisionPointRec(mouse, btnControles)) {
            *gameState = GAME_PAUSE_CONTROLS;
        }
    }
}
