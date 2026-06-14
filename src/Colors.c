#include <float.h>
#include <math.h>
#include "raylib/raylib.h"

//PICO-8 extended palette
unsigned char palette[32][3] = {
    //Official base colors
    {0, 0, 0},          // black
    {29, 43, 83},       // dark-blue
    {126, 37, 83},      // dark-purple
    {0, 135, 81},       // dark-green
    {171, 82, 54},      // brown
    {95, 87, 79},       // dark-grey
    {194, 195, 199},    // light-grey
    {255, 241, 232},    // white
    {255, 0, 77},       // red
    {255, 163, 0},      // orange
    {255, 236, 39},     // yellow
    {0, 228, 54},       // green
    {41, 173, 255},     // blue
    {131, 118, 156},    // lavender
    {255, 119, 168},    // pink
    {255, 204, 170},    // light-peach

    //Undocumented extra colors
    {41, 24, 20},       // brownish-black
    {17, 29, 53},       // darker-blue
    {66, 33, 59},       // darker-purple
    {18, 83, 89},       // blue-green
    {116, 47, 41},      // dark-brown
    {73, 51, 59},       // darker-grey
    {162, 136, 121},    // medium-grey
    {243, 239, 125},    // light-yellow
    {190, 18, 80},      // dark-red
    {255, 108, 36},     // dark-orange
    {168, 231, 46},     // lime-green
    {0, 181, 67},       // medium-green
    {6, 90, 181},       // true-blue
    {117, 70, 101},     // mauve
    {255, 110, 89},     // dark-peach
    {255, 157, 129}     // peach
};

Color getPaletteMatch(Color color) {
    Color bestMatch;
    float minDistance = FLT_MAX;

    for(int i = 0; i < 32; i++) {
        Color paletteColor = (Color){palette[i][0], palette[i][1], palette[i][2], 255};
        float distance = sqrt(pow(color.r - paletteColor.r, 2) + pow(color.g - paletteColor.g, 2) + pow(color.b - paletteColor.b, 2));

        if(distance < minDistance) {
            bestMatch = paletteColor;
            minDistance = distance;
        }
    }

    return bestMatch;
}

/**
 * @brief Interpolates the progress between the specified start and end colors
 */
Color interpolateColor(Color start, Color end, float progress) {
    unsigned char lerpR = start.r + (unsigned char)((end.r - start.r) * progress);
    unsigned char lerpG = start.g + (unsigned char)((end.g - start.g) * progress);
    unsigned char lerpB = start.b + (unsigned char)((end.b - start.b) * progress);
    unsigned char lerpA = start.a + (unsigned char)((end.a - start.a) * progress);

    return getPaletteMatch((Color){lerpR, lerpG, lerpB, lerpA});
}