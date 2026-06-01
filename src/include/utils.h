#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

/**
 * @brief Clamps a number between the specified values
 */
int clamp(int value, int min, int max);

/**
 * @brief Interpolates the progress between the specified start and end floats
 */
float interpolateFloat(float start, float end, float progress);

/**
 * @brief Draws text with an outline based on position and font size
 */
void drawOutlinedText(const char *text, int posX, int posY, int fontSize, struct Color color, struct Color outlineColor);

#endif