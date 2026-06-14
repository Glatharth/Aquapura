#ifndef ENUMS_H
#define ENUMS_H

#define MAX_PLAYERS 2000

typedef enum State {
    GAME_MENU,
    GAME_RUNNING,
    GAME_PAUSED,
    GAME_OVER,
    GAME_CREDITS,
    GAME_CONTROLS,
    GAME_UNKNOWN
} State;

typedef enum Direction {
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef enum NPCType {
    NPC_ANIMAL = 0,
    NPC_GARBAGE = 1,
    NPC_BUBBLE = 2
} NPCType;

typedef enum InputEvent {
    INPUT_UI_ESCAPE,
    INPUT_UI_UP,
    INPUT_UI_LEFT,
    INPUT_UI_DOWN,
    INPUT_UI_RIGHT,
    INPUT_UI_SELECT,
    INPUT_UI_CLICK,
    INPUT_P1_MOVE_UP,
    INPUT_P1_MOVE_LEFT,
    INPUT_P1_MOVE_DOWN,
    INPUT_P1_MOVE_RIGHT,
    INPUT_P1_CAPTURE
} InputEvent;

typedef enum GameMode {
    MODE_HUMAN_ONLY,
    MODE_AI_TRAINING,
    MODE_AI_PLAY,
    MODE_HUMAN_AND_AI
} GameMode;

typedef enum {
    ACAO_ESPERAR = 0,
    ACAO_CIMA = 1,
    ACAO_BAIXO = 2,
    ACAO_FRENTE = 3,
    ACAO_TRAS = 4,
    ACAO_CAPTURAR = 5,
    ACAO_MORRER = 6
} AcaoBot;

#endif