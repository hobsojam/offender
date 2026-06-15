#include "humanoid.h"
#include <cmath>

void Humanoid::init(float worldX, float terrainY) {
    wx          = worldX;
    groundY     = terrainY;
    y           = groundY;
    vy          = 0.f;
    alive       = true;
    falling     = false;
    beingCarried = false;
    carrierIdx  = -1;
}

void Humanoid::update(float dt) {
    if (!alive || beingCarried) return;
    if (falling) {
        vy += 200.f * dt;  // gravity
        if (vy > HUM_FALL_SPD) vy = HUM_FALL_SPD;
        y += vy * dt;
        if (y >= groundY) {
            y = groundY;
            vy = 0.f;
            falling = false;
        }
    }
}

void Humanoid::draw(float camX, float shakeX, float shakeY) const {
    if (!alive) return;
    float sx = wsX(wx, camX) + shakeX;
    float sy = y - HUM_H + shakeY;  // top of humanoid

    // Little stick figure: head + body
    DrawCircle((int)sx, (int)(sy + 3.f), 3.f, {255, 200, 150, 255});
    DrawLine((int)sx, (int)(sy + 6.f), (int)sx, (int)(sy + HUM_H - 2.f), {255, 200, 150, 255});
    DrawLine((int)(sx - 3.f), (int)(sy + 8.f),  (int)(sx + 3.f), (int)(sy + 8.f),  {255, 200, 150, 255});
    DrawLine((int)(sx - 3.f), (int)(sy + HUM_H), (int)sx, (int)(sy + HUM_H - 4.f), {255, 200, 150, 255});
    DrawLine((int)(sx + 3.f), (int)(sy + HUM_H), (int)sx, (int)(sy + HUM_H - 4.f), {255, 200, 150, 255});
}
