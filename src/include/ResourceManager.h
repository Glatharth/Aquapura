/**
 * @file ResourceManager.h
 * @author Prof. Dr. David Buzatto
 * @brief ResourceManager struct and function declarations.
 * 
 * @copyright Copyright (c) 2025
 */
#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "raylib/raylib.h"

typedef struct ResourceManager {
    //Misc. Images
    Image icon;

    //Player
    Texture2D player;
    Texture2D playerAttacking;
    Texture2D oxyTank;

    //NPCs
    Texture2D animalArray[17];
    Texture2D enemyArray[11];

    Texture2D bubbleIdle;
    Texture2D bubblePop;
    Texture2D bubbleBreathe;

    //Game BGs
    Texture2D sunBg;
    Texture2D moonBg;
    Texture2D cloudHighlightBg;
    Texture2D cloudShadowBg;
    Texture2D cityscapeBg;
    Texture2D cityOverlayBg;
    Texture2D waterBg;
    Texture2D floorBg;

    //Game FGs
    Texture2D foamFg;
    Texture2D bubbleFg;
    Texture2D floorFg;

    //Menus
    Texture2D menuBg; 
    Texture2D menuCredits;
    Texture2D menuControls;
    Texture2D menuGameOver;
    Texture2D menuPause;

    //Botões
    Texture2D controlsButton;
    Texture2D playButton;
    Texture2D creditsButton;
    Texture2D backButton;
    Texture2D menuButton;
    Texture2D againButton;
    Texture2D backButton2;

    //Sound soundExample;
    
    Music bg_tune;
    
} ResourceManager;

/**
 * @brief Global ResourceManager instance.
 */
extern ResourceManager rm;

/**
 * @brief Load global game resources, linking them in the global instance of
 * ResourceManager called rm.
 */
void loadResourcesResourceManager( void );

/**
 * @brief Unload global game resources.
 */
void unloadResourcesResourceManager( void );

#endif