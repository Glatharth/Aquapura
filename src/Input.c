#include "Input.h"

bool inputEvents[RECOGNIZED_INPUTS] = {0};

void sendInputEvent(InputEvent input) {
    inputEvents[input] = true;
}

bool consumeInputEvent(InputEvent input) {
    bool state = inputEvents[input];
    inputEvents[input] = false;
    return state;
}