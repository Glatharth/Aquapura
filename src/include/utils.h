#ifndef UTILS_H
#define UTILS_H

#include "raylib/raylib.h"

/**
 * @brief Clamps a number between the specified values
 */
int clamp(int value, int min, int max);

/**
 * @brief Interpolates the progress between the specified start and end colors
 */
Color interpolateColor(Color start, Color end, float progress);

/**
 * @brief Draws text with an outline based on position and font size
 */
void drawOutlinedText(const char *text, int posX, int posY, int fontSize, Color color, Color outlineColor);

#endif