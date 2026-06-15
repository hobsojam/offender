#pragma once
#include "entity.h"

struct Sprites;  // forward — full def in sprites.h

struct Humanoid {
    float wx;         // world X
    float groundY;    // resting screen Y (on terrain)
    float y;          // current screen Y
    float vy;         // fall velocity (when falling)
    bool  alive;
    bool  falling;        // dropped by lander, in free fall
    bool  beingCarried;   // attached to a lander
    bool  targeted;       // a lander is descending toward this humanoid
    int   carrierIdx;     // index in enemies array (-1 if none)

    void init(float worldX, float terrrainY);
    void update(float dt);
    void draw(float camX, float shakeX, float shakeY, const Sprites& spr) const;
};
