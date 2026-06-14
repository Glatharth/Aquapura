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

// Regras de ambiente dinâmicas controladas pela Julia
float globalBubbleOxygen = 25.0f;
float globalSurfaceRegen = 6.0f;
float globalMissedPenalty = 0.0f;

void setEnvironmentRules(float bubbleOxy, float surfaceReg, float penaltyMissedGbg) {
    globalBubbleOxygen = bubbleOxy;
    globalSurfaceRegen = surfaceReg;
    globalMissedPenalty = penaltyMissedGbg;
}
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

GameMode currentMode = MODE_HUMAN_ONLY;
static GameWorld *instanciaGlobalMundo = NULL;

GameWorld* createGameWorld(void) {
    GameWorld *gw = calloc(1, sizeof(*gw));
    instanciaGlobalMundo = gw; 
    
    gw->spawnInterval = INITIAL_SPAWN_INTERVAL;
    gw->npcSpeed = 60;
    gw->probAnimal = 50;
    gw->probGarbage = 50;

    setGameMode(MODE_HUMAN_ONLY); 
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

// Funções antigas de radar foram removidas. A filtragem agora ocorre na Julia.

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

        if (IsKeyPressed(KEY_F6)) setGameMode(MODE_HUMAN_ONLY);
        if (IsKeyPressed(KEY_F7)) setGameMode(MODE_AI_TRAINING);
        if (IsKeyPressed(KEY_F8)) setGameMode(MODE_AI_PLAY);
        if (IsKeyPressed(KEY_F9)) setGameMode(MODE_HUMAN_AND_AI);
        
        if (IsKeyPressed(KEY_EIGHT)) {
            gw->npcSpeed = fmax(60, gw->npcSpeed - 10);
            gw->spawnInterval = fmin(INITIAL_SPAWN_INTERVAL * 2, gw->spawnInterval + 0.1f);
            gw->probGarbage = 50;
            gw->probAnimal = 50;
        }
        if (IsKeyPressed(KEY_NINE)) {
            gw->npcSpeed = fmin(MAX_NPC_SPEED, gw->npcSpeed + 10);
            gw->spawnInterval = fmax(MIN_SPAWN_INTERVAL, gw->spawnInterval - 0.1f);
            gw->probGarbage = 50;
            gw->probAnimal = 50;
        }
        if (IsKeyPressed(KEY_ZERO)) {
            gw->npcSpeed = 60;
            gw->spawnInterval = INITIAL_SPAWN_INTERVAL;
            gw->probGarbage = 50;
            gw->probAnimal = 50;
        }
        // ========================================================
        
        static double *vetorEstados = NULL;
        static int *vetorAcoes = NULL;
        static float *colisoesNesteFrame = NULL;
        if (!vetorEstados) {
            // Buffer de memória para todos os NPCs e Jogadores
            int tamanhoMaximo = 2 + (MAX_NPC * 4) + (MAX_PLAYERS * 4);
            vetorEstados = (double*)malloc(tamanhoMaximo * sizeof(double));
            vetorAcoes = (int*)malloc(MAX_PLAYERS * sizeof(int));
            colisoesNesteFrame = (float*)calloc(MAX_PLAYERS, sizeof(float));
        }

        int idxEstados = 0;
        
        // 1. Despeja informações de todos os NPCs vivos
        int idxNpcCount = idxEstados++; // Guarda a posição para escrever o total
        int actualNpcs = 0;
        
        for(int i = 0; i < MAX_NPC; i++) {
            if(gw->npc[i] != NULL && !gw->npc[i]->shouldBeRemoved) {
                vetorEstados[idxEstados++] = gw->npc[i]->type;
                vetorEstados[idxEstados++] = gw->npc[i]->collision.x;
                vetorEstados[idxEstados++] = gw->npc[i]->collision.y;
                vetorEstados[idxEstados++] = gw->npc[i]->speed.x; 
                actualNpcs++;
            }
        }
        vetorEstados[idxNpcCount] = actualNpcs; // Atualiza com o valor real

        // 2. Despeja informações de todos os Jogadores (IAs)
        vetorEstados[idxEstados++] = gw->maxJogadoresAtuais;
        for(int i = 0; i < gw->maxJogadoresAtuais; i++) {
            Player *p = gw->jogadores[i];
            
            if(p != NULL && p->oxygen > 0) {
                vetorEstados[idxEstados++] = p->collision.x;
                vetorEstados[idxEstados++] = p->collision.y;
                vetorEstados[idxEstados++] = p->oxygen;
                vetorEstados[idxEstados++] = colisoesNesteFrame[i]; 
                
                colisoesNesteFrame[i] = 0.0f;
            } else {
                vetorEstados[idxEstados++] = -1.0;
                vetorEstados[idxEstados++] = -1.0;
                vetorEstados[idxEstados++] = -1.0;
                vetorEstados[idxEstados++] = -1.0;
                colisoesNesteFrame[i] = 0.0f;
            }
        }

        if (currentMode != MODE_HUMAN_ONLY) {
            obterDecisoesDaPopulacao(vetorEstados, idxEstados, gw->maxJogadoresAtuais, vetorAcoes);
        }

        bool todosMortos = true;

        for(int i = 0; i < gw->maxJogadoresAtuais; i++) {
            Player *p = gw->jogadores[i];
            
            if(p != NULL && p->oxygen > 0) {
                todosMortos = false;
                
                bool isAi = false;
                if (currentMode == MODE_AI_TRAINING || currentMode == MODE_AI_PLAY) isAi = true;
                if (currentMode == MODE_HUMAN_AND_AI && i == 1) isAi = true;
                
                AcaoBot acao = isAi ? (AcaoBot)vetorAcoes[i] : ACAO_ESPERAR;
                
                // ACAO_MORRER (6) é forçada pelo Julia (Timeout ou Suicídio intencional)
                if (acao == ACAO_MORRER) {
                    p->oxygen = 0;
                } else {
                    updatePlayer(p, delta, isAi, acao);
                }
            }
        }

        if(gw->spawnTimer > gw->spawnInterval){
            gw->spawnTimer = 0.0f;
            if(gw->activeNpc < MAX_NPC) {
                for (int i = 0; i < MAX_NPC; i++) {
                    if (gw->npc[i] == NULL) {
                        int npcSpeed = randomSpeed ? fmin(gw->npcSpeed + GetRandomValue(0, 100), MAX_NPC_SPEED) : gw->npcSpeed;
                        
                        NPCType typeToSpawn;
                        if (onlySpawnGarbage) {
                            typeToSpawn = NPC_GARBAGE;
                        } else {
                            int totalProb = gw->probAnimal + gw->probGarbage;
                            if (totalProb <= 0) totalProb = 100;
                            int randVal = GetRandomValue(1, totalProb);
                            if (randVal <= gw->probAnimal) typeToSpawn = NPC_ANIMAL;
                            else typeToSpawn = NPC_GARBAGE;
                        }
                        
                        gw->npc[i] = createNpc(npcSpeed, typeToSpawn);
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
                            if (currentMode == MODE_AI_TRAINING && gw->npc[i]->capturedBy[j]) {
                                continue;
                            }

                            bool oldRemoved = gw->npc[i]->shouldBeRemoved;
                            checkNpcCapture(gw, p, gw->npc[i]);
                            if (!gw->npc[i]->shouldBeRemoved) {
                                checkNpcCollision(p, gw->npc[i]);
                            }
                            
                            if(!oldRemoved && gw->npc[i]->shouldBeRemoved) {
                                float penalty_or_bonus = gw->npc[i]->captureScore * 100.0f;
                                
                                colisoesNesteFrame[j] += penalty_or_bonus;
                                
                                if (currentMode == MODE_AI_TRAINING) {
                                    gw->npc[i]->shouldBeRemoved = false;
                                    gw->npc[i]->capturedBy[j] = true;
                                } else {
                                    break; 
                                }
                            }
                        }
                    }

                    if(gw->npc[i] != NULL && gw->npc[i]->collision.x + gw->npc[i]->collision.width + 16 < 0){
                        gw->npc[i]->shouldBeRemoved = true;
                        gw->npc[i]->removalCountdown = 0;

                        if(gw->npc[i]->type == NPC_GARBAGE) {
                            gw->escapedEnemies++;
                            
                            // Aplica penalidade a todas as IAs ativas se deixarem o lixo fugir
                            if (currentMode != MODE_HUMAN_ONLY && globalMissedPenalty > 0.0f) {
                                for(int j = 0; j < gw->maxJogadoresAtuais; j++) {
                                    if(gw->jogadores[j] != NULL && gw->jogadores[j]->oxygen > 0) {
                                        if(!gw->npc[i]->capturedBy[j]) {
                                            colisoesNesteFrame[j] -= globalMissedPenalty; 
                                        }
                                    }
                                }
                            }
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
                            p->oxygen = fmin(p->oxygen + globalSurfaceRegen * delta, MAX_OXYGEN);
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
            if (currentMode == MODE_AI_TRAINING) {
                notificarFimDeGeracao();
                resetarGameWorld(gw->maxJogadoresAtuais);
            } else {
                setGameState(additionalData, GAME_OVER);
            }
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
            
            // --- DESENHA O ID FLUTUANTE ---
            char txtScore[16];
            sprintf(txtScore, "ID: %d", i);
            
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

void setGameDifficulty(float interval, int speed) {
    if (instanciaGlobalMundo == NULL) return;
    instanciaGlobalMundo->spawnInterval = interval;
    instanciaGlobalMundo->npcSpeed = speed;
}

void setSpawnProbabilities(int probAnimal, int probGarbage) {
    if (instanciaGlobalMundo == NULL) return;
    instanciaGlobalMundo->probAnimal = probAnimal;
    instanciaGlobalMundo->probGarbage = probGarbage;
}

void setGameMode(GameMode mode) {
    currentMode = mode;
    if (mode == MODE_HUMAN_ONLY) {
        resetarGameWorld(1);
        SetTargetFPS(60);
    }
    else if (mode == MODE_AI_TRAINING) {
        extern int obterQtdJogadoresJulia();
        resetarGameWorld(obterQtdJogadoresJulia());
        SetTargetFPS(0); // Acelera o jogo apenas no modo de treinamento
    }
    else if (mode == MODE_AI_PLAY) {
        resetarGameWorld(1);
        SetTargetFPS(60);
    }
    else if (mode == MODE_HUMAN_AND_AI) {
        resetarGameWorld(2);
        SetTargetFPS(60);
    }
}