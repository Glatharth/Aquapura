#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"
#include "GameWorld.h"
#include "GameWindow.h"
#include "GlobalVariables.h"
#include "ResourceManager.h"
#include "Utils.h"
#include "Enums.h"
#include "Colors.h"
#include "Player.h"
#include "Npc.h"
#include "GameMechanics.h"
#include "Score.h"
#include "Input.h"
#include "DefinitionJulia.h" 

const int globalPixelWidth = 320;
const int globalPixelHeight = 180;
const int globalWaterSurfaceHeight = 60;
const int globalFloorHeight = 172;

bool hudVisible = true;
bool pauseOxygen = false;
bool randomSpeed = true;
bool drawPlayerAsFish = false;
bool onlySpawnGarbage = false;

static GameWorld *instanciaGlobalMundo = NULL;

GameWorld* createGameWorld(void) {
    GameWorld *gw = calloc(1, sizeof(*gw));
    instanciaGlobalMundo = gw; 
    
    gw->spawnInterval = INITIAL_SPAWN_INTERVAL;
    gw->npcSpeed = 60;

    resetarGameWorld(50); 
    return gw;
}

void destroyGameWorld(GameWorld **gw) {
    if(gw == NULL || *gw == NULL) return;

    for(int i = 0; i < MAX_NPC; i++) {
        if((*gw)->npc[i] != NULL) free((*gw)->npc[i]);
    }
    for(int i = 0; i < MAX_PLAYERS; i++) {
        if((*gw)->jogadores[i] != NULL) free((*gw)->jogadores[i]);
    }
    free(*gw);
    *gw = NULL;
}

void resetarGameWorld(int qtdPlayers) {
    if (instanciaGlobalMundo == NULL) return;
    GameWorld *gw = instanciaGlobalMundo;

    for(int i = 0; i < MAX_PLAYERS; i++) {
        if(gw->jogadores[i] != NULL) {
            free(gw->jogadores[i]);
            gw->jogadores[i] = NULL;
        }
        gw->pontuacoes[i] = 0.0;
    }

    for(int i = 0; i < MAX_NPC; i++) {
        if(gw->npc[i] != NULL) {
            free(gw->npc[i]);
            gw->npc[i] = NULL;
        }
    }
    
    gw->activeNpc = 0;
    gw->gameTime = 0;
    gw->spawnTimer = 0;
    gw->previousTime = 0;
    gw->bubbleTimer = 0;
    gw->escapedEnemies = 0;
    gw->caughtEnemies = 0;
    gw->npcSpeed = 60;
    gw->spawnInterval = INITIAL_SPAWN_INTERVAL;

    int limit = (qtdPlayers > MAX_PLAYERS) ? MAX_PLAYERS : qtdPlayers;
    for(int i = 0; i < limit; i++) {
        gw->jogadores[i] = createPlayer();
    }
    gw->numJogadoresAtivos = limit;
    gw->maxJogadoresAtuais = limit;
}

float calcularDistanciaInimigoMaisProximo(GameWorld *gw, Player *p) {
    if (!p) return 1000.0f;
    float menor_distancia = 9999.0f;
    for(int i = 0; i < MAX_NPC; i++) {
        if(gw->npc[i] != NULL && gw->npc[i]->type == NPC_GARBAGE) {
            float dist = gw->npc[i]->collision.x - (p->collision.x + p->collision.width);
            if(dist > 0 && dist < menor_distancia) {
                menor_distancia = dist;
            }
        }
    }
    return menor_distancia == 9999.0f ? 1000.0f : menor_distancia;
}

float calcularDistanciaBolhaMaisProxima(GameWorld *gw, Player *p) {
    if (!p) return 1000.0f;
    float menor_distancia = 9999.0f;
    for(int i = 0; i < MAX_NPC; i++) {
        if(gw->npc[i] != NULL && gw->npc[i]->type == NPC_BUBBLE) {
            float dist = gw->npc[i]->collision.x - (p->collision.x + p->collision.width);
            if(dist > 0 && dist < menor_distancia) {
                menor_distancia = dist;
            }
        }
    }
    return menor_distancia == 9999.0f ? 1000.0f : menor_distancia;
}

void updateGameWorld(void *gameWorld, float delta, void* additionalData) {
    GameWorld *gw = (GameWorld*)gameWorld;

    if(consumeInputEvent(INPUT_UI_ESCAPE)) {
        setGameState(additionalData, GAME_PAUSED);
    }
    else {
        if (gw->maxJogadoresAtuais <= 0) return; 

        gw->previousTime = gw->gameTime;
        gw->gameTime += delta;
        gw->spawnTimer += delta;
        gw->bubbleTimer += delta;

        // ========================================================
        // DEBUG: MATAR TODOS OS CLONES (TECLA K)
        // ========================================================
        if (IsKeyPressed(KEY_K)) {
            for(int i = 0; i < gw->maxJogadoresAtuais; i++) {
                if(gw->jogadores[i] != NULL) {
                    gw->jogadores[i]->oxygen = 0.0f;
                }
            }
            gw->numJogadoresAtivos = 0;
        }
        // ========================================================
        
        double vetorEstados[MAX_PLAYERS * 5];
        int vetorAcoes[MAX_PLAYERS];

        for(int i = 0; i < gw->maxJogadoresAtuais; i++) {
            Player *p = gw->jogadores[i];
            
            if(p != NULL && p->oxygen > 0) {
                vetorEstados[i*5 + 0] = p->oxygen;
                vetorEstados[i*5 + 1] = calcularDistanciaInimigoMaisProximo(gw, p);
                vetorEstados[i*5 + 2] = calcularDistanciaBolhaMaisProxima(gw, p);
                vetorEstados[i*5 + 3] = gw->pontuacoes[i]; 
                vetorEstados[i*5 + 4] = 1.0; 
            } else {
                vetorEstados[i*5 + 0] = 0.0;
                vetorEstados[i*5 + 1] = 0.0;
                vetorEstados[i*5 + 2] = 0.0;
                vetorEstados[i*5 + 3] = gw->pontuacoes[i];
                vetorEstados[i*5 + 4] = 0.0; 
            }
        }

        obterDecisoesDaPopulacao(vetorEstados, gw->maxJogadoresAtuais, vetorAcoes);

        bool todosMortos = true;

        for(int i = 0; i < gw->maxJogadoresAtuais; i++) {
            Player *p = gw->jogadores[i];
            
            if(p != NULL && p->oxygen > 0) {
                todosMortos = false;
                updatePlayer(p, delta, true, (AcaoBot)vetorAcoes[i]);
                gw->pontuacoes[i] += delta * 10.0;
            }
        }

        if(gw->spawnTimer > gw->spawnInterval){
            gw->spawnTimer = 0.0f;
            if(gw->activeNpc < MAX_NPC) {
                for (int i = 0; i < MAX_NPC; i++) {
                    if (gw->npc[i] == NULL) {
                        int npcSpeed = randomSpeed ? fmin(gw->npcSpeed + GetRandomValue(0, 100), MAX_NPC_SPEED) : gw->npcSpeed;
                        gw->npc[i] = createNpc(npcSpeed);
                        gw->activeNpc++;
                        break;                   
                    }
                }
            }
        }

        if(gw->bubbleTimer > BUBBLE_SPAWN_INTERVAL){
            gw->bubbleTimer = 0.0f;
            if(gw->activeNpc < MAX_NPC) {
                for(int i = 0; i < MAX_NPC; i++) {
                    if(gw->npc[i] == NULL) {
                        gw->npc[i] = createBubble(68);
                        gw->activeNpc++;
                        break;
                    }
                }
            }
        }

        if(gw->escapedEnemies >= ENEMY_ESCAPE_LIMIT || gw->caughtEnemies >= ENEMY_CAUGHT_LIMIT) {
            gw->npcSpeed = fmin(gw->npcSpeed + 4, MAX_NPC_SPEED);
            gw->spawnInterval = fmax(MIN_SPAWN_INTERVAL, gw->spawnInterval - SPAWN_DECREMENT);
            gw->escapedEnemies = 0;
            gw->caughtEnemies = 0;
        }

        for(int i = 0; i < MAX_NPC; i++) {
            if(gw->npc[i] != NULL) {
                updateNpc(gw->npc[i], delta);

                if(gw->npc[i]->shouldBeRemoved) {
                    if(gw->npc[i]->removalCountdown <= 0) {
                        free(gw->npc[i]);
                        gw->npc[i] = NULL;
                        gw->activeNpc--;
                        continue;
                    }
                }
                else {
                    for(int j = 0; j < gw->maxJogadoresAtuais; j++) {
                        Player *p = gw->jogadores[j];
                        if(p != NULL && p->oxygen > 0) {
                            checkNpcCapture(gw, p, gw->npc[i]);
                            checkNpcCollision(p, gw->npc[i]);
                            
                            if(gw->npc[i]->shouldBeRemoved) {
                                gw->pontuacoes[j] += 500.0;
                                break; 
                            }
                        }
                    }

                    if(gw->npc[i] != NULL && gw->npc[i]->collision.x + gw->npc[i]->collision.width + 16 < 0){
                        gw->npc[i]->shouldBeRemoved = true;
                        gw->npc[i]->removalCountdown = 0;

                        if(gw->npc[i]->type == NPC_GARBAGE) {
                            gw->escapedEnemies++;
                        }
                    }
                }
            }
        }

        if(!pauseOxygen) {
            for(int i = 0; i < gw->maxJogadoresAtuais; i++) {
                Player *p = gw->jogadores[i];
                if(p != NULL && p->oxygen > 0){
                    if(p->collision.y == globalWaterSurfaceHeight && p->netTimer == 0) {
                        if(p->oxygen < MAX_OXYGEN) {
                            p->oxygen = fmin(p->oxygen + 6 * delta, MAX_OXYGEN);
                        }
                    }
                    else if(p->collision.y + p->collision.height / 2 < (globalPixelHeight * 2 / 3)) {
                        p->oxygen -= 3 * delta;
                    }
                    else {
                        p->oxygen -= 6 * delta;
                    }

                    if(p->oxygen <= 0) { 
                        p->oxygen = 0;
                        gw->numJogadoresAtivos--;
                    }
                }
            }
        }
        
        if (todosMortos || gw->numJogadoresAtivos <= 0) {
            notificarFimDeGeracao(gw->pontuacoes, gw->maxJogadoresAtuais);
        }
    }
}

void drawGameWorld(void *gameWorld, float alpha, void* additionalData) {
    GameWorld *gw = (GameWorld*)gameWorld;
    float animTime = interpolateFloat(gw->previousTime, gw->gameTime, alpha);

    drawBackground(animTime);

    for(int i = 0; i < gw->maxJogadoresAtuais; i++) {
        if(gw->jogadores[i] != NULL && gw->jogadores[i]->oxygen > 0) {
            // Desenha o peixe/mergulhador
            drawPlayer(gw->jogadores[i], alpha, animTime);
            
            // --- DESENHA A PONTUAÇÃO FLUTUANTE ---
            char txtScore[16];
            sprintf(txtScore, "%.0f", gw->pontuacoes[i]);
            
            // Calcula a posição centralizada acima da cabeça
            Vector2 textSize = MeasureTextEx(GetFontDefault(), txtScore, 10, 1);
            int x = roundf((gw->jogadores[i]->collision.x + gw->jogadores[i]->collision.width / 2) - (textSize.x / 2)) * currentWindowScale;
            int y = roundf(gw->jogadores[i]->collision.y - 12) * currentWindowScale; // -12 sobe o texto um pouco
            
            // Usa a função nativa do seu template para desenhar com borda (Fica lindo na água!)
            drawOutlinedText(txtScore, x, y, 10 * currentWindowScale, YELLOW, BLACK);
        }
    }

    for (int i = 0; i < MAX_NPC; i++) {
        if(gw->npc[i] != NULL) {
            if(gw->npc[i]->type == NPC_BUBBLE) {
                drawBubble(gw->npc[i], alpha);
            }
            else {
                drawNpc(gw->npc[i], alpha);
            }
        }
    }

    drawForeground(animTime);

    if(hudVisible) {
        for(int i = 0; i < gw->maxJogadoresAtuais; i++) {
            if(gw->jogadores[i] != NULL && gw->jogadores[i]->oxygen > 0) {
                drawOxygenBar(gw->jogadores[i]);
                break;
            }
        }
    }
}

void drawBackground(float animTime) {
    Texture2D *celestial;
    Color skyColor;
    Color cloudShadowColor;
    Color cloudHighlightColor;
    Color cityscapeColor;
    Color cityOverlayColor;
    Color waterColor;

    int dayTimeInterval = 60;
    float colorLerpProgress = fmin(fmax(0, fmod(animTime, dayTimeInterval) - dayTimeInterval + 1), 1);

    switch((int)(animTime / dayTimeInterval) % 4) {
        case 1:
            celestial = &rm.sunBg;
            skyColor = interpolateColor(PICO_8_DARK_ORANGE, PICO_8_DARK_BLUE, colorLerpProgress);
            cloudHighlightColor = interpolateColor(PICO_8_WHITE, PICO_8_LIGHT_GREY, colorLerpProgress);
            cloudShadowColor = interpolateColor(PICO_8_LIGHT_PEACH, PICO_8_LAVENDER, colorLerpProgress);
            cityscapeColor = interpolateColor(PICO_8_DARKER_GREY, PICO_8_BLACK, colorLerpProgress);
            cityOverlayColor = interpolateColor(PICO_8_DARKER_GREY, PICO_8_LIGHT_YELLOW, colorLerpProgress);
            waterColor = interpolateColor(PICO_8_BLUE_GREEN, PICO_8_DARKER_BLUE, colorLerpProgress);
            break;
        case 2:
            celestial = &rm.moonBg;
            skyColor = interpolateColor(PICO_8_DARK_BLUE, PICO_8_MAUVE, colorLerpProgress);
            cloudHighlightColor = interpolateColor(PICO_8_LIGHT_GREY, PICO_8_PEACH, colorLerpProgress);
            cloudShadowColor = interpolateColor(PICO_8_LAVENDER, PICO_8_DARK_PEACH, colorLerpProgress);
            cityscapeColor = interpolateColor(PICO_8_BLACK, PICO_8_DARKER_GREY, colorLerpProgress);
            cityOverlayColor = PICO_8_LIGHT_YELLOW;
            waterColor = interpolateColor(PICO_8_DARKER_BLUE, PICO_8_BLUE_GREEN, colorLerpProgress);
            break;
        case 3:
            celestial = &rm.moonBg;
            skyColor = interpolateColor(PICO_8_MAUVE, PICO_8_BLUE, colorLerpProgress);
            cloudHighlightColor = interpolateColor(PICO_8_PEACH, PICO_8_WHITE, colorLerpProgress);
            cloudShadowColor = interpolateColor(PICO_8_DARK_PEACH, PICO_8_LIGHT_GREY, colorLerpProgress);
            cityscapeColor = interpolateColor(PICO_8_DARKER_GREY, PICO_8_TRUE_BLUE, colorLerpProgress);
            cityOverlayColor = interpolateColor(PICO_8_LIGHT_YELLOW, PICO_8_TRUE_BLUE, colorLerpProgress);
            waterColor = PICO_8_BLUE_GREEN;
            break;
        default:
            celestial = &rm.sunBg;
            skyColor = interpolateColor(PICO_8_BLUE, PICO_8_DARK_ORANGE, colorLerpProgress);
            cloudHighlightColor = PICO_8_WHITE;
            cloudShadowColor = interpolateColor(PICO_8_LIGHT_GREY, PICO_8_LIGHT_PEACH, colorLerpProgress);
            cityscapeColor = interpolateColor(PICO_8_TRUE_BLUE, PICO_8_DARKER_GREY, colorLerpProgress);
            cityOverlayColor = interpolateColor(PICO_8_TRUE_BLUE, PICO_8_DARKER_GREY, colorLerpProgress);
            waterColor = PICO_8_BLUE_GREEN;
    }

    Rectangle source;
    Rectangle dest;
    Vector2 offset = {0, 0};

    DrawRectangle(0, 0, GetScreenWidth(), 64 * currentWindowScale, skyColor);

    float celestialAngle = (fmod((animTime + 1) / dayTimeInterval / 2, 1)) * PI;
    int celestialX = (int)(globalPixelWidth * (0.5 - 0.4 * cosf(celestialAngle))) * currentWindowScale;
    int celestialY = (int)(globalPixelHeight * (0.5 - 0.3 * sinf(celestialAngle))) * currentWindowScale;
    source = (Rectangle){0, 0, 32, 32};
    dest = (Rectangle){celestialX, celestialY, 32 * currentWindowScale, 32 * currentWindowScale};
    DrawTexturePro(*celestial, source, dest, (Vector2){16 * currentWindowScale, 24 * currentWindowScale}, 0, WHITE);

    source = (Rectangle){0, 0, 320, 64};
    dest = (Rectangle){0, 0, GetScreenWidth(), 64 * currentWindowScale};
    DrawTexturePro(rm.cloudShadowBg, source, dest, offset, 0, cloudShadowColor);
    DrawTexturePro(rm.cloudHighlightBg, source, dest, offset, 0, cloudHighlightColor);

    source = (Rectangle){roundf(animTime), 0, 320, 180};
    dest = (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()};
    DrawTexturePro(rm.cityscapeBg, source, dest, offset, 0, cityscapeColor);
    DrawTexturePro(rm.cityOverlayBg, source, dest, offset, 0, cityOverlayColor);

    dest = (Rectangle){0, 52 * currentWindowScale, GetScreenWidth(), 128 * currentWindowScale};
    source = (Rectangle){320 + roundf(animTime * 70), 0, 320, 128};
    DrawTexturePro(rm.waterBg, source, dest, offset, 0, waterColor);
    source.height /= 4;
    dest.height /= 4;
    DrawTexturePro(rm.foamFg, source, dest, offset, 0, WHITE);
    source = (Rectangle){roundf(animTime * 90), 0, 320, 128};
    dest.height *= 4;
    DrawTexturePro(rm.waterBg, source, dest, offset, 0, waterColor);

    source = (Rectangle){roundf(animTime * 40), 0, 320, 64};
    dest = (Rectangle){0, 116 * currentWindowScale, GetScreenWidth(), 64 * currentWindowScale};
    DrawTexturePro(rm.floorBg, source, dest, offset, 0, WHITE);
}

void drawForeground(float animTime) {
    Rectangle source;
    Rectangle dest;
    Vector2 offset = {0, 0};

    source = (Rectangle){roundf(animTime * 90), 0, 320, 32};
    dest = (Rectangle){0, 52 * currentWindowScale, GetScreenWidth(), 32 * currentWindowScale};
    DrawTexturePro(rm.foamFg, source, dest, offset, 0, WHITE);

    source = (Rectangle){roundf(animTime * 100), 0, 320, 128};
    dest = (Rectangle){0, 52 * currentWindowScale, GetScreenWidth(), 128 * currentWindowScale};
    DrawTexturePro(rm.bubbleFg, source, dest, offset, 0, WHITE);

    source = (Rectangle){roundf(animTime * 110), 0, 320, 32};
    dest = (Rectangle){0, 148 * currentWindowScale, GetScreenWidth(), 32 * currentWindowScale};
    DrawTexturePro(rm.floorFg, source, dest, offset, 0, WHITE);
}

void toggleHUD(void) {
    hudVisible = !hudVisible;
}

void toggleOxygen(void) {
    pauseOxygen = !pauseOxygen;
}

void toggleRandomSpeed(void) {
    randomSpeed = !randomSpeed;
}

void toggleFishPlayer(void) {
    drawPlayerAsFish = !drawPlayerAsFish;
}

void toggleAnimals(void) {
    onlySpawnGarbage = !onlySpawnGarbage;
}

void setInterval(GameWorld *gw, float t) {
    gw->spawnInterval = t;
}