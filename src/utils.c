#include "utils.h"

/**
 * @brief Clamps a number between the specified values
 */
int clamp(int value, int min, int max) {
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

/**
 * @brief Interpolates the progress between the specified start and end colors
 */
Color interpolateColor(Color start, Color end, float progress) {
    int lerpR = start.r + (int)((end.r - start.r) * progress);
    int lerpG = start.g + (int)((end.g - start.g) * progress);
    int lerpB = start.b + (int)((end.b - start.b) * progress);
    int lerpA = start.a + (int)((end.a - start.a) * progress);

    return (Color){lerpR, lerpG, lerpB, lerpA};
}

/**
 * @brief Draws text with an outline based on position and font size
 */
void drawOutlinedText(const char *text, int posX, int posY, int fontSize, Color color, Color outlineColor) {
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