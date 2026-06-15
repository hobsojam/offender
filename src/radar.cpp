#include "radar.h"
#include <cmath>

static constexpr float RADAR_Y      = 0.f;
static constexpr float RADAR_X_PAD  = 4.f;
static constexpr float RADAR_W_DRAW = SCREEN_W - RADAR_X_PAD * 2.f;

static float toRadarX(float wx) {
    return RADAR_X_PAD + (wrapX(wx) / WORLD_W) * RADAR_W_DRAW;
}

static float toRadarY(float sy) {
    // Map screen Y (PLAY_TOP..SCREEN_H) to radar strip height
    float t = (sy - PLAY_TOP) / (SCREEN_H - PLAY_TOP);
    return RADAR_Y + 4.f + t * (RADAR_H - 8.f);
}

void DrawRadar(float camX, float playerWX, float playerSY,
               const Enemy* enemies, int enemyCount,
               const Humanoid* hums, int humCount,
               const float* terrain)
{
    // Background
    DrawRectangle(0, (int)RADAR_Y, SCREEN_W, (int)RADAR_H, {0, 0, 0, 220});
    DrawLine(0, (int)(RADAR_Y + RADAR_H - 1), SCREEN_W, (int)(RADAR_Y + RADAR_H - 1), {0, 180, 0, 255});

    // Terrain line in radar (compressed)
    for (int i = 0; i < TERR_NODES; i++) {
        float wx0 = (float)i * TERR_SEG;
        float rx   = toRadarX(wx0);
        float ry   = toRadarY(terrain[i]);
        DrawPixel((int)rx, (int)ry, {0, 150, 0, 255});
    }

    // Humanoids (green dots)
    for (int i = 0; i < humCount; i++) {
        if (!hums[i].alive) continue;
        float rx = toRadarX(hums[i].wx);
        float ry = toRadarY(hums[i].y);
        DrawPixel((int)rx, (int)ry, {0, 255, 100, 255});
    }

    // Enemies
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].alive) continue;
        float rx = toRadarX(enemies[i].wpos.x);
        float ry = toRadarY(enemies[i].wpos.y);
        Color col = {255, 80, 80, 255};
        if (enemies[i].type == EnemyType::MUTANT) col = {200, 80, 255, 255};
        if (enemies[i].type == EnemyType::BAITER) col = {255, 220, 0,  255};
        DrawPixel((int)rx, (int)ry, col);
        DrawPixel((int)rx+1, (int)ry, col);
    }

    // Player (bright white dot)
    float prx = toRadarX(playerWX);
    float pry = toRadarY(playerSY);
    DrawRectangle((int)prx - 1, (int)pry - 1, 3, 3, WHITE);

    // Viewport box
    float vx0 = toRadarX(camX);
    float vx1 = toRadarX(wrapX(camX + SCREEN_W));
    if (vx1 > vx0) {
        DrawRectangleLines((int)vx0, (int)(RADAR_Y + 2.f),
                           (int)(vx1 - vx0), (int)(RADAR_H - 4.f),
                           {255, 255, 255, 80});
    }

    // Label
    DrawText("RADAR", SCREEN_W - 58, (int)(RADAR_Y + 2.f), 10, {0, 200, 0, 200});
}
