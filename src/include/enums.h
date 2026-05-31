#ifndef ENUMS_H
#define ENUMS_H

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
    INPUT_UI_ESCAPE = 0,
    INPUT_UI_UP = 1,
    INPUT_UI_LEFT = 2,
    INPUT_UI_DOWN = 3,
    INPUT_UI_RIGHT = 4,
    INPUT_UI_SELECT = 5,
    INPUT_P1_MOVE_UP = 6,
    INPUT_P1_MOVE_LEFT = 7,
    INPUT_P1_MOVE_DOWN = 8,
    INPUT_P1_MOVE_RIGHT = 9,
    INPUT_P1_CAPTURE = 10,
} InputEvent;

#endif