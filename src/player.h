#pragma once
#include "entity.h"

struct Player {
    Vector2 pos;          // world X, screen Y
    Vector2 vel;
    bool    facingRight;
    bool    alive;
    float   laserCD;
    float   invTimer;     // invincibility after respawn
    float   enginePhase;  // for engine glow animation

    void  init(float worldX, float groundY);
    void  handleInput(float dt);
    void  update(float dt, float groundY);
    void  draw(float camX, float shakeX, float shakeY) const;

    bool  canFire() const  { return laserCD <= 0.f; }
    bool  invincible() const { return invTimer > 0.f; }
    // AABB in world/screen mixed coords for collision
    float wx() const { return pos.x; }
    float sy() const { return pos.y; }
};
