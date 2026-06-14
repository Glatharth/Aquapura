/**
 * @file main.c
 * @author Prof. Dr. David Buzatto
 * @brief Main function and logic for the game. Base template for game
 * development in C using Raylib (https://www.raylib.com/).
 * * @copyright Copyright (c) 2025
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "GameWindow.h"
#include "ConfigJulia.h"
#include "DefinitionJulia.h"

int main(void) {
    if (juliaInit() != 0) {
        printf("Erro fatal: Falha ao iniciar a Julia.\n");
        return 1;
    }

    if (!prepararModelosIA()) {
        printf("Erro fatal: Nao foi possivel cachear as funcoes.\n");
        return 1;
    }

    GameWindow *gameWindow = createGameWindow(
        640,                    // width
        360,                    // height
        "Operação Aquapura",    // title
        60,                     // target FPS
        false,                  // antialiasing
        false,                  // resizable
        false,                  // full screen
        false,                  // undecorated
        false,                  // always on top
        false,                  // invisible background
        false,                  // always run
        true,                   // load resources
        true                    // init audio
    );

    initGameWindow( gameWindow );

    juliaClose();

    return 0;
}