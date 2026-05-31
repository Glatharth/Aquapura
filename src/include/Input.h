#include <stdbool.h>
#include "Enums.h"

#define RECOGNIZED_INPUTS 11

void sendInputEvent(InputEvent input);
bool consumeInputEvent(InputEvent input);