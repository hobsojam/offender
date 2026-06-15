#include "player.h"
#include "sprites.h"
#include <cmath>

void Player::init(float worldX, float groundY) {
    pos        = { worldX, groundY - P_H * 0.5f - 4.f };
    vel        = { 0.f, 0.f };
    facingRight = true;
    alive       = true;
    laserCD     = 0.f;
    invTimer    = INVINCIBLE_TIME;
    enginePhase = 0.f;
    carryHumIdx = -1;
}

void Player::handleInput(float dt) {
    if (!alive) return;

    bool left  = IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A);
    bool right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
    bool up    = IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W);
    bool down  = IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S);

    if (right) {
        vel.x += P_ACCEL * dt;
        facingRight = true;
    } else if (left) {
        vel.x -= P_ACCEL * dt;
        facingRight = false;
    } else {
        // friction
        float friction = P_FRICTION * dt;
        if (absf(vel.x) <= friction) vel.x = 0.f;
        else                          vel.x -= signf(vel.x) * friction;
    }

    if (up)   vel.y -= P_VY * dt * 3.5f;
    if (down) vel.y += P_VY * dt * 3.5f;

    // Vertical damping
    if (!up && !down) {
        float vfric = P_FRICTION * dt * 1.5f;
        if (absf(vel.y) <= vfric) vel.y = 0.f;
        else                       vel.y -= signf(vel.y) * vfric;
    }

    vel.x = clampf(vel.x, -P_MAX_VX, P_MAX_VX);
    vel.y = clampf(vel.y, -P_VY, P_VY);

    if (laserCD > 0.f) laserCD -= dt;
}

void Player::update(float dt, float groundY) {
    if (!alive) return;

    pos.x = wrapX(pos.x + vel.x * dt);
    pos.y += vel.y * dt;
    pos.y = clampf(pos.y, P_MIN_Y, minf(P_MAX_Y, groundY - P_H * 0.5f - 2.f));

    if (invTimer > 0.f) invTimer -= dt;
    enginePhase += dt * 8.f;
}

void Player::draw(float camX, float shakeX, float shakeY, const Sprites& spr) const {
    if (!alive) return;
    if (invTimer > 0.f && (int)(invTimer * 8.f) % 2 == 0) return;

    float sx = wsX(pos.x, camX) + shakeX;
    float sy = pos.y + shakeY;

    // Sprite at 1.5× scale; negative source width = horizontal flip for left
    float dw = 30.f, dh = 18.f;
    float tw = (float)spr.player.width;
    float th = (float)spr.player.height;
    Rectangle src  = { facingRight ? 0.f : tw, 0.f,
                        facingRight ? tw  : -tw, th };
    Rectangle dest = { sx - dw * 0.5f, sy - dh * 0.5f, dw, dh };
    DrawTexturePro(spr.player, src, dest, {0.f, 0.f}, 0.f, WHITE);

    // Animated engine glow drawn on top of the sprite
    float dir  = facingRight ? 1.f : -1.f;
    float glow = 0.5f + 0.5f * sinf(enginePhase);
    unsigned char gb = (unsigned char)(180.f + glow * 75.f);
    DrawCircle((int)(sx - dir * 13.f), (int)sy, 3.5f + glow * 2.f,
               {gb, gb, 255, 220});
}
