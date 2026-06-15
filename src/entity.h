#pragma once
#include "raylib.h"
#include "config.h"
#include <cmath>

inline float wrapX(float x) {
    x = fmodf(x, WORLD_W);
    if (x < 0.f) x += WORLD_W;
    return x;
}

// Shortest signed delta on the wrapped X axis
inline float wrapDX(float from, float to) {
    float d = to - from;
    if (d >  WORLD_W * 0.5f) d -= WORLD_W;
    if (d < -WORLD_W * 0.5f) d += WORLD_W;
    return d;
}

// World X → screen X given camX (left edge of viewport)
inline float wsX(float wx, float camX) {
    float sx = wx - camX;
    if (sx < -WORLD_W * 0.5f) sx += WORLD_W;
    if (sx >  WORLD_W * 0.5f) sx -= WORLD_W;
    return sx;
}

inline float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
inline float signf(float v)  { return v > 0.f ? 1.f : v < 0.f ? -1.f : 0.f; }
inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline float absf(float v)   { return v < 0.f ? -v : v; }
inline float minf(float a, float b) { return a < b ? a : b; }
inline float maxf(float a, float b) { return a > b ? a : b; }

struct Particle {
    Vector2 pos, vel;
    float   life, maxLife, size;
    Color   col;
    bool    active;
};

enum class EnemyType  { LANDER, MUTANT, BAITER };
enum class LanderState { WANDERING, DESCENDING, GRABBING, CARRYING, ASCENDED };
