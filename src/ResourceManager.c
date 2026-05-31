/**
 * @file ResourceManager.c
 * @author Prof. Dr. David Buzatto
 * @brief ResourceManager implementation.
 * 
 * @copyright Copyright (c) 2025
 */
#include <stddef.h>
#include <string.h>

#include "raylib/raylib.h"

#include "ResourceManager.h"
#include "resources.h"

#define LOAD_MUSIC_STREAM_FROM_MEMORY(dest, src) { \
    rm.dest = LoadMusicStreamFromMemory(".mp3", _binary_resources_sounds_##src##_mp3_start, (size_t)(_binary_resources_sounds_##src##_mp3_end - _binary_resources_sounds_##src##_mp3_start)); \
}

#define LOAD_TEXTURE_FROM_IMAGE(dest, src) { \
    Image img = LoadImageFromMemory(".png", _binary_resources_images_##src##_png_start, (size_t)(_binary_resources_images_##src##_png_end - _binary_resources_images_##src##_png_start)); \
    rm.dest = LoadTextureFromImage(img); \
    UnloadImage(img); \
}

ResourceManager rm = {0};

void loadResourcesResourceManager(void) {
    //Player
    LOAD_TEXTURE_FROM_IMAGE(player, sprites_diver);
    LOAD_TEXTURE_FROM_IMAGE(playerAttacking, sprites_diver_attacking);
    LOAD_TEXTURE_FROM_IMAGE(oxyTank, sprites_tank);

    //NPCs
    LOAD_TEXTURE_FROM_IMAGE(animalArray[0], sprites_npc_animal_animal_0);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[1], sprites_npc_animal_animal_1);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[2], sprites_npc_animal_animal_2);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[3], sprites_npc_animal_animal_3);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[4], sprites_npc_animal_animal_4);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[5], sprites_npc_animal_animal_5);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[6], sprites_npc_animal_animal_6);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[7], sprites_npc_animal_animal_7);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[8], sprites_npc_animal_animal_8);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[9], sprites_npc_animal_animal_9);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[10], sprites_npc_animal_animal_10);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[11], sprites_npc_animal_animal_11);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[12], sprites_npc_animal_animal_12);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[13], sprites_npc_animal_animal_13);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[14], sprites_npc_animal_animal_14);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[15], sprites_npc_animal_animal_15);
    LOAD_TEXTURE_FROM_IMAGE(animalArray[16], sprites_npc_animal_animal_16);

    LOAD_TEXTURE_FROM_IMAGE(enemyArray[0], sprites_npc_enemy_trash_0);
    LOAD_TEXTURE_FROM_IMAGE(enemyArray[1], sprites_npc_enemy_trash_1);
    LOAD_TEXTURE_FROM_IMAGE(enemyArray[2], sprites_npc_enemy_trash_2);
    LOAD_TEXTURE_FROM_IMAGE(enemyArray[3], sprites_npc_enemy_trash_3);
    LOAD_TEXTURE_FROM_IMAGE(enemyArray[4], sprites_npc_enemy_trash_4);
    LOAD_TEXTURE_FROM_IMAGE(enemyArray[5], sprites_npc_enemy_trash_5);
    LOAD_TEXTURE_FROM_IMAGE(enemyArray[6], sprites_npc_enemy_trash_6);
    LOAD_TEXTURE_FROM_IMAGE(enemyArray[7], sprites_npc_enemy_trash_7);
    LOAD_TEXTURE_FROM_IMAGE(enemyArray[8], sprites_npc_enemy_trash_8);
    LOAD_TEXTURE_FROM_IMAGE(enemyArray[9], sprites_npc_enemy_trash_9);
    LOAD_TEXTURE_FROM_IMAGE(enemyArray[10], sprites_npc_enemy_trash_10);
    
    LOAD_TEXTURE_FROM_IMAGE(bubbleIdle, sprites_npc_bubble_bubble_idle);
    LOAD_TEXTURE_FROM_IMAGE(bubblePop, sprites_npc_bubble_bubble_pop);
    LOAD_TEXTURE_FROM_IMAGE(bubbleBreathe, sprites_npc_bubble_bubble_breathe);

    //Game BGs
    LOAD_TEXTURE_FROM_IMAGE(sunBg, sprites_background_sunbg);
    LOAD_TEXTURE_FROM_IMAGE(moonBg, sprites_background_moonbg);
    LOAD_TEXTURE_FROM_IMAGE(cloudHighlightBg, sprites_background_cloudhighlightbg);
    LOAD_TEXTURE_FROM_IMAGE(cloudShadowBg, sprites_background_cloudshadowbg);
    LOAD_TEXTURE_FROM_IMAGE(cityscapeBg, sprites_background_cityscapebg);
    LOAD_TEXTURE_FROM_IMAGE(cityOverlayBg, sprites_background_cityoverlaybg);
    LOAD_TEXTURE_FROM_IMAGE(waterBg, sprites_background_waterbg);
    LOAD_TEXTURE_FROM_IMAGE(floorBg, sprites_background_floorbg);

    //Game FGs
    LOAD_TEXTURE_FROM_IMAGE(foamFg, sprites_background_foamfg);
    LOAD_TEXTURE_FROM_IMAGE(bubbleFg, sprites_background_bubblefg);
    LOAD_TEXTURE_FROM_IMAGE(floorFg, sprites_background_floorfg);

    //Menus
    LOAD_TEXTURE_FROM_IMAGE(menuBg, ui_menu_bg);
    LOAD_TEXTURE_FROM_IMAGE(menuCredits, ui_menu_crdts);
    LOAD_TEXTURE_FROM_IMAGE(menuControls, ui_menu_ctrls);
    LOAD_TEXTURE_FROM_IMAGE(menuGameOver, ui_menu_gmov);
    LOAD_TEXTURE_FROM_IMAGE(menuPause, ui_menu_pause);

    //Buttons
    LOAD_TEXTURE_FROM_IMAGE(controlsButton, ui_bt_controles);
    LOAD_TEXTURE_FROM_IMAGE(playButton, ui_bt_jogar);
    LOAD_TEXTURE_FROM_IMAGE(creditsButton, ui_bt_creditos);
    LOAD_TEXTURE_FROM_IMAGE(backButton, ui_bt_voltar);
    LOAD_TEXTURE_FROM_IMAGE(menuButton, ui_bt_menu);
    LOAD_TEXTURE_FROM_IMAGE(againButton, ui_bt_denovo);
    LOAD_TEXTURE_FROM_IMAGE(backButton2, ui_bt_voltar2);

    //Music
    LOAD_MUSIC_STREAM_FROM_MEMORY(bg_tune, music_hydrodynamics);
}

void unloadResourcesResourceManager(void) {
    //Player
    UnloadTexture(rm.player);
    UnloadTexture(rm.playerAttacking);
    UnloadTexture(rm.oxyTank);

    //NPCs
    for(int i = 0; i < 17; i++) {
        UnloadTexture(rm.animalArray[i]);
    }
    for(int i = 0; i < 11; i++) {
        UnloadTexture(rm.enemyArray[i]);
    }
    UnloadTexture(rm.bubbleIdle);
    UnloadTexture(rm.bubblePop);
    UnloadTexture(rm.bubbleBreathe);

    //Game BGs
    UnloadTexture(rm.sunBg);
    UnloadTexture(rm.moonBg);
    UnloadTexture(rm.cloudHighlightBg);
    UnloadTexture(rm.cloudShadowBg);
    UnloadTexture(rm.cityscapeBg);
    UnloadTexture(rm.cityOverlayBg);
    UnloadTexture(rm.waterBg);
    UnloadTexture(rm.floorBg);

    //Game FGs
    UnloadTexture(rm.foamFg);
    UnloadTexture(rm.bubbleFg);
    UnloadTexture(rm.floorFg);

    //Menus
    UnloadTexture(rm.menuBg);
    UnloadTexture(rm.menuCredits);
    UnloadTexture(rm.menuControls);
    UnloadTexture(rm.menuGameOver);
    UnloadTexture(rm.menuPause);

    //Buttons
    UnloadTexture(rm.controlsButton);
    UnloadTexture(rm.playButton);
    UnloadTexture(rm.creditsButton);
    UnloadTexture(rm.backButton);
    UnloadTexture(rm.menuButton);
    UnloadTexture(rm.againButton);
    UnloadTexture(rm.backButton2);

    UnloadMusicStream(rm.bg_tune);
}