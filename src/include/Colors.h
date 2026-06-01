#ifndef COLORS_H
#define COLORS_H

#include "raylib/raylib.h"

#define PICO_8_BLACK CLITERAL(Color){0, 0, 0, 255}
#define PICO_8_DARK_BLUE CLITERAL(Color){29, 43, 83, 255}
#define PICO_8_DARK_PURPLE CLITERAL(Color){126, 37, 83, 255}
#define PICO_8_DARK_GREEN CLITERAL(Color){0, 135, 81, 255}
#define PICO_8_BROWN CLITERAL(Color){171, 82, 54, 255}
#define PICO_8_DARK_GREY CLITERAL(Color){95, 87, 79, 255}
#define PICO_8_LIGHT_GREY CLITERAL(Color){194, 195, 199, 255}
#define PICO_8_WHITE CLITERAL(Color){255, 241, 232, 255}
#define PICO_8_RED CLITERAL(Color){255, 0, 77, 255}
#define PICO_8_ORANGE CLITERAL(Color){255, 163, 0, 255}
#define PICO_8_YELLOW CLITERAL(Color){255, 236, 39, 255}
#define PICO_8_GREEN CLITERAL(Color){0, 228, 54, 255}
#define PICO_8_BLUE CLITERAL(Color){41, 173, 255, 255}
#define PICO_8_LAVENDER CLITERAL(Color){131, 118, 156, 255}
#define PICO_8_PINK CLITERAL(Color){255, 119, 168, 255}
#define PICO_8_LIGHT_PEACH CLITERAL(Color){255, 204, 170, 255}
#define PICO_8_BROWNISH_BLACK CLITERAL(Color){41, 24, 20, 255}
#define PICO_8_DARKER_BLUE CLITERAL(Color){17, 29, 53, 255}
#define PICO_8_DARKER_PURPLE CLITERAL(Color){66, 33, 59, 255}
#define PICO_8_BLUE_GREEN CLITERAL(Color){18, 83, 89, 255}
#define PICO_8_DARK_BROWN CLITERAL(Color){116, 47, 41, 255}
#define PICO_8_DARKER_GREY CLITERAL(Color){73, 51, 59, 255}
#define PICO_8_MEDIUM_GREY CLITERAL(Color){162, 136, 121, 255}
#define PICO_8_LIGHT_YELLOW CLITERAL(Color){243, 239, 125, 255}
#define PICO_8_DARK_RED CLITERAL(Color){190, 18, 80, 255}
#define PICO_8_DARK_ORANGE CLITERAL(Color){255, 108, 36, 255}
#define PICO_8_LIME_GREEN CLITERAL(Color){168, 231, 46, 255}
#define PICO_8_MEDIUM_GREEN CLITERAL(Color){0, 181, 67, 255}
#define PICO_8_TRUE_BLUE CLITERAL(Color){6, 90, 181, 255}
#define PICO_8_MAUVE CLITERAL(Color){117, 70, 101, 255}
#define PICO_8_DARK_PEACH CLITERAL(Color){255, 110, 89, 255}
#define PICO_8_PEACH CLITERAL(Color){255, 157, 129, 255}

/**
 * @brief Gets the closest matching color in the palette.
 */
Color getPaletteMatch(Color color);

/**
 * @brief Interpolates the progress between the specified start and end colors
 */
struct Color interpolateColor(struct Color start, struct Color end, float progress);

#endif