#include <stdbool.h>
#include "Enums.h"

#define RECOGNIZED_INPUTS 12

void sendInputEvent(InputEvent input);
bool consumeInputEvent(InputEvent input);