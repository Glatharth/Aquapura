#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <stdbool.h>
#include <Enums.h>

//Set in GameWindow.c
extern const bool debug;
extern int currentWindowScale;

//Set in GameWorld.c
extern const int globalPixelWidth;
extern const int globalPixelHeight;
extern const int globalWaterSurfaceHeight;
extern const int globalFloorHeight;

//Set in Score.c
extern int score;
extern int hiscore;

#endif