#pragma once
#include "entity.h"

struct Humanoid;  // forward

struct Enemy {
    EnemyType   type;
    Vector2     wpos;      // world X, screen Y
    Vector2     vel;
    LanderState lstate;    // LANDER only
    int         humIdx;    // LANDER: index of target/carried humanoid (-1 = none)
    float       shotCD;
    float       wobble;    // phase angle for sine wobble
    float       stateCD;   // misc state timer
    bool        alive;

    void  initLander(float wx, float y);
    void  initMutant(float wx, float y);
    void  initBaiter(float wx, float y);

    void  update(float dt, const Vector2& playerWPos,
                 Humanoid* hums, int humCount, float terrainY,
                 // out: request to fire a shot
                 Vector2* shotOut, bool* wantShot);

    void  draw(float camX, float shakeX, float shakeY) const;

    float halfW() const;
    float halfH() const;

private:
    void updateLander(float dt, const Vector2& pWPos,
                      Humanoid* hums, int humCount, float terrainY,
                      Vector2* shotOut, bool* wantShot);
    void updateMutant(float dt, const Vector2& pWPos,
                      Vector2* shotOut, bool* wantShot);
    void updateBaiter(float dt, const Vector2& pWPos,
                      Vector2* shotOut, bool* wantShot);
};
