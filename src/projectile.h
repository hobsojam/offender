#pragma once
#include "entity.h"

struct Laser {
    float wx;    // world X center
    float y;     // screen Y center
    float vx;    // world-space horizontal velocity
    float life;
    bool  active;
};

struct Shot {
    Vector2 wpos;  // world X, screen Y
    Vector2 vel;
    float   life;
    bool    active;
};
