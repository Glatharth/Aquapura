#include "raylib/raylib.h"
#include "Utils.h"
#include "Colors.h"

/**
 * @brief Clamps a number between the specified values
 */
int clamp(int value, int min, int max) {
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

/**
 * @brief Interpolates the progress between the specified start and end floats
 */
float interpolateFloat(float start, float end, float progress) {
    return start + (end - start) * progress;
}

/**
 * @brief Draws text with an outline based on position and font size
 */
void drawOutlinedText(const char *text, int posX, int posY, int fontSize, struct Color color, struct Color outlineColor) {
    int offset = fontSize / 10;

    DrawText(text, posX - offset, posY - offset, fontSize, outlineColor);
    DrawText(text, posX, posY - offset, fontSize, outlineColor);
    DrawText(text, posX + offset, posY - offset, fontSize, outlineColor);
    DrawText(text, posX - offset, posY, fontSize, outlineColor);
    DrawText(text, posX + offset, posY, fontSize, outlineColor);
    DrawText(text, posX - offset, posY + offset, fontSize, outlineColor);
    DrawText(text, posX, posY + offset, fontSize, outlineColor);
    DrawText(text, posX + offset, posY + offset, fontSize, outlineColor);
    DrawText(text, posX, posY, fontSize, color);
}