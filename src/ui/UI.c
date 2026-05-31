#include <stdlib.h>
#include "raylib/raylib.h"
#include "UI.h"
#include "Input.h"
#include "GlobalVariables.h"

Menu *buildMenu(int buttonCount) {
    Menu *m = malloc(sizeof(*m) + sizeof(*m->buttons) * buttonCount);
    m->escapeAction = NULL;
    m->targetState = GAME_MENU;
    m->selectedButton = 0;
    m->buttonCount = buttonCount;
    return m;
}

void destroyMenu(Menu **m) {
    if(m == NULL || *m == NULL) return;

    for(int i = 0; i < (*m)->buttonCount; i++) {
        free((*m)->buttons[i]);
    }
    free(*m);
    *m = NULL;
}

Button *createButton(Texture2D *texture, Rectangle bounds, void (*action)(void*, State), State targetState) {
    Button *b = malloc(sizeof(*b));
    b->texture = texture;
    b->bounds = bounds;
    b->selected = false;
    b->action = action;
    b->targetState = targetState;
    return b;
}

void updateGenericMenu(void *menu, float delta, void* additionalData) {
    Menu *m = (Menu*)menu;

    Vector2 clickPos = GetMousePosition();
    for(int i = 0; i < m->buttonCount; i++) {
        Button *b = (Button*)m->buttons[i];
        if(b != NULL && b->action != NULL) {
            Rectangle bounds = {
                b->bounds.x * currentWindowScale,
                b->bounds.y * currentWindowScale,
                b->bounds.width * currentWindowScale,
                b->bounds.height * currentWindowScale
            };

            b->selected = CheckCollisionPointRec(clickPos, bounds);
            if(b->selected && consumeInputEvent(INPUT_UI_SELECT)) {
                b->action(additionalData, b->targetState);
                b->selected = false;
            }
        }
    }
    if(m->escapeAction != NULL && consumeInputEvent(INPUT_UI_ESCAPE)) m->escapeAction(additionalData, m->targetState);
}

void drawGenericMenu(void *menu, float alpha, void* additionalData) {
    Menu *m = (Menu*)menu;

    drawMenuBackground(m);
    drawMenuButtons(m);
}

void drawButton(Button *b) {
    Rectangle source = {0, 0, (float)b->texture->width, (float)b->texture->height};
    Rectangle dest;

    if(b->selected) {
        float scale = 1.1;
        dest = (Rectangle){
            (b->bounds.x - (b->bounds.width * (scale - 1.0f) / 2.0f)) * currentWindowScale,
            (b->bounds.y - (b->bounds.height * (scale - 1.0f) / 2.0f)) * currentWindowScale,
            b->bounds.width * scale * currentWindowScale,
            b->bounds.height * scale * currentWindowScale
        };
    }
    else {
        dest = (Rectangle){
            b->bounds.x * currentWindowScale,
            b->bounds.y * currentWindowScale,
            b->bounds.width * currentWindowScale,
            b->bounds.height * currentWindowScale
        };
    }

    DrawTexturePro(*b->texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

void drawMenuBackground(Menu *m) {
    DrawTexturePro (
        *m->background,
        (Rectangle){0, 0, (float)m->background->width, (float)m->background->height},
        (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}

void drawMenuButtons(Menu *m) {
    for(int i = 0; i < m->buttonCount; i++) {
        drawButton(m->buttons[i]);
    }
}