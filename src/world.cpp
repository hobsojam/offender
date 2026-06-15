#include "world.h"
#include "random.h"
#include <cmath>

void World::init() {
    camX = 0.f;
    float p1 = RandomFloat(0.f, 2.f * PI);
    float p2 = RandomFloat(0.f, 2.f * PI);
    float p3 = RandomFloat(0.f, 2.f * PI);
    for (int i = 0; i < TERR_NODES; i++) {
        float t = (float)i / TERR_NODES * 2.f * PI;
        float y = GROUND_BASE
            + GROUND_AMP * 0.50f * sinf(t * 3.f + p1)
            + GROUND_AMP * 0.30f * sinf(t * 7.f + p2)
            + GROUND_AMP * 0.20f * sinf(t * 13.f + p3);
        terrain[i] = y;
    }
    for (int i = 0; i < NUM_STARS; i++)
        stars[i] = { RandomFloat(0.f, WORLD_W), RandomFloat(PLAY_TOP + 15.f, 550.f) };
}

float World::terrainY(float wx) const {
    wx = wrapX(wx);
    float fi = wx / TERR_SEG;
    int i0 = (int)fi % TERR_NODES;
    int i1 = (i0 + 1) % TERR_NODES;
    float t = fi - (float)(int)fi;
    return lerpf(terrain[i0], terrain[i1], t);
}

void World::updateCamera(float playerWX, bool facingRight, float dt) {
    float lead = facingRight ? SCREEN_W * 0.20f : -SCREEN_W * 0.20f;
    float target = playerWX - SCREEN_W * 0.5f + lead;
    float diff = wrapDX(camX, target);
    camX = wrapX(camX + diff * (1.f - expf(-10.f * dt)));
}

void World::draw() const {
    // Stars
    for (int i = 0; i < NUM_STARS; i++) {
        float sx = wsX(stars[i].x, camX);
        if (sx >= 0.f && sx < (float)SCREEN_W)
            DrawPixel((int)sx, (int)stars[i].y, {150, 150, 200, 255});
    }

    // Terrain: start from the segment containing camX so the wraparound case
    // (viewport spanning world-end → world-start) renders correctly.
    int startSeg = (int)floorf(camX / TERR_SEG) - 1;
    int numSegs  = (int)(SCREEN_W / TERR_SEG) + 4;

    for (int k = startSeg; k <= startSeg + numSegs; k++) {
        int i = ((k % TERR_NODES) + TERR_NODES) % TERR_NODES;
        int j = (i + 1) % TERR_NODES;

        float wx0 = (float)k * TERR_SEG;
        float wx1 = wx0 + TERR_SEG;
        float sx0 = wx0 - camX;
        float sx1 = wx1 - camX;

        if (sx1 < -4.f || sx0 > SCREEN_W + 4.f) continue;

        float y0 = terrain[i];
        float y1 = terrain[j];

        // Per-column fill (simple and correct)
        int xA = (int)floorf(maxf(sx0, 0.f));
        int xB = (int)ceilf (minf(sx1, (float)SCREEN_W));
        float denom = sx1 - sx0;
        for (int x = xA; x < xB; x++) {
            float frac = (denom > 0.001f) ? (x - sx0) / denom : 0.f;
            int yTop = (int)lerpf(y0, y1, frac);
            DrawLine(x, yTop, x, SCREEN_H, {0, 50, 0, 255});
        }
        // Bright surface line
        DrawLineEx({sx0, y0}, {sx1, y1}, 2.f, {0, 220, 0, 255});
    }
}
