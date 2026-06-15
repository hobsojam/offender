#include "humanoid.h"
#include "sprites.h"
#include <cmath>

void Humanoid::init(float worldX, float terrainY) {
    wx          = worldX;
    groundY     = terrainY;
    y           = groundY;
    vy          = 0.f;
    alive           = true;
    falling         = false;
    beingCarried    = false;
    carriedByPlayer = false;
    targeted        = false;
    carrierIdx      = -1;
}

void Humanoid::update(float dt) {
    if (!alive || beingCarried || carriedByPlayer) return;
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

void Humanoid::draw(float camX, float shakeX, float shakeY, const Sprites& spr) const {
    if (!alive) return;
    float sx = wsX(wx, camX) + shakeX;
    float sy = y - HUM_H + shakeY;  // top of sprite = top of humanoid AABB

    Rectangle src  = {0.f, 0.f, (float)spr.humanoid.width, (float)spr.humanoid.height};
    Rectangle dest = {sx - HUM_W * 0.5f, sy, HUM_W, HUM_H};
    DrawTexturePro(spr.humanoid, src, dest, {0.f, 0.f}, 0.f, WHITE);
}
