#include <stdio.h>

#include "raylib/raylib.h"

#include "Score.h"
#include "GlobalVariables.h"
#include "Enums.h"
#include "Utils.h"

int score = 0;
int hiscore = 0;

bool isNewBest = false;

void drawInGameScore(void) {
    char scoreText[11];
    sprintf(scoreText, "%d", score);

    Vector2 textOffset = MeasureTextEx(GetFontDefault(), scoreText, 10, 1);
    textOffset.x = (int)textOffset.x;
    textOffset.y = (int)(textOffset.y / 2);

    int x = (globalPixelWidth - 16 - textOffset.x) * currentWindowScale;
    int y = (globalPixelHeight - 16 - textOffset.y) * currentWindowScale;
    drawOutlinedText(scoreText, x, y, 10 * currentWindowScale, WHITE, BLACK);
}

void drawBestScore(bool showLastScore) {
    Vector2 textOffset;
    int x;
    int y;

    Color color = WHITE;
    char bestScoreText[32];
    sprintf(bestScoreText, "Melhor: %d", hiscore);

    if(showLastScore) {
        char scoreText[11];
        sprintf(scoreText, "%d", score);

        textOffset = MeasureTextEx(GetFontDefault(), scoreText, 10, 1);
        textOffset.x = (int)(textOffset.x / 2);
        textOffset.y = (int)(textOffset.y / 2);
        x = (globalPixelWidth / 2 - textOffset.x) * currentWindowScale;
        y = (globalPixelHeight - 32 - textOffset.y) * currentWindowScale;

        if(isNewBest) {
            color = GREEN;
            sprintf(bestScoreText, "Novo melhor!");
        }

        drawOutlinedText(scoreText, x, y, 10 * currentWindowScale, WHITE, BLACK);
    }

    textOffset = MeasureTextEx(GetFontDefault(), bestScoreText, 10, 1);
    textOffset.x = (int)(textOffset.x / 2);
    textOffset.y = (int)(textOffset.y / 2);

    x = (globalPixelWidth / 2 - textOffset.x) * currentWindowScale;
    y = (globalPixelHeight - 16 - textOffset.y) * currentWindowScale;
    drawOutlinedText(bestScoreText, x, y, 10 * currentWindowScale, color, BLACK);
}

void updateBestScore(void) {
    if(score > hiscore) {
        hiscore = score;
        isNewBest = true;
    }
    else {
        isNewBest = false;
    }
}