#pragma once
#include "raylib.h"

struct Sprites {
    Texture2D player;    // 20×12, facing right — flip src width for left
    Texture2D lander;    // 16×14
    Texture2D mutant;    // 14×14
    Texture2D baiter;    // 16×8
    Texture2D humanoid;  // 8×14

    void load();
    void unload();
};
