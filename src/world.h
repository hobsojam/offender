#pragma once
#include "entity.h"

struct World {
    float   terrain[TERR_NODES];
    float   camX;
    Vector2 stars[NUM_STARS];

    void  init();
    void  updateCamera(float playerWX, bool facingRight, float dt);
    float terrainY(float wx) const;
    void  draw() const;
};
