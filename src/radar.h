#pragma once
#include "entity.h"
#include "enemies.h"
#include "humanoid.h"

void DrawRadar(float camX, float playerWX, float playerSY,
               const Enemy* enemies, int enemyCount,
               const Humanoid* hums, int humCount,
               const float* terrain);
