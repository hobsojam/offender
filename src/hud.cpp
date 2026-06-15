#include "hud.h"
#include "raylib.h"
#include "config.h"
#include <cstdio>

void DrawHUD(int score, int hiScore, int lives, int wave, int bombs) {
    float y = RADAR_H + 4.f;

    // Score (left)
    char buf[64];
    snprintf(buf, sizeof(buf), "%07d", score);
    DrawText(buf, 8, (int)y, 20, {0, 230, 0, 255});

    // Hi-score (centre)
    snprintf(buf, sizeof(buf), "HI %07d", hiScore);
    int tw = MeasureText(buf, 20);
    DrawText(buf, (SCREEN_W - tw) / 2, (int)y, 20, {255, 200, 0, 255});

    // Wave (right)
    snprintf(buf, sizeof(buf), "WAVE %d", wave);
    int ww = MeasureText(buf, 20);
    DrawText(buf, SCREEN_W - ww - 8, (int)y, 20, {100, 200, 255, 255});

    // Lives (ships, row below score)
    float ly = y + 26.f;
    for (int i = 0; i < lives && i < 6; i++) {
        float lx = 8.f + i * 22.f;
        // Mini ship icon
        DrawTriangle({lx + 14.f, ly + 6.f},
                     {lx,        ly + 2.f},
                     {lx,        ly + 10.f}, {180, 220, 255, 255});
        DrawRectangle((int)lx - 8, (int)(ly + 2.f), 10, 8, {200, 200, 220, 200});
    }

    // Bombs (B icons)
    for (int i = 0; i < bombs && i < MAX_BOMBS; i++) {
        float bx = SCREEN_W - 8.f - (i + 1) * 20.f;
        DrawCircle((int)bx, (int)(ly + 6.f), 6.f, {255, 100, 50, 255});
        DrawText("B", (int)(bx - 4.f), (int)(ly + 1.f), 11, BLACK);
    }
}
