#include "enemies.h"
#include "humanoid.h"
#include "sprites.h"
#include <cmath>
#include <cstdlib>

static float randf(float lo, float hi) {
    return lo + (float)rand() / (float)RAND_MAX * (hi - lo);
}

float Enemy::halfW() const {
    switch (type) {
        case EnemyType::LANDER: return LAND_W * 0.5f;
        case EnemyType::MUTANT: return MUTT_W * 0.5f;
        case EnemyType::BAITER: return BAIT_W * 0.5f;
    }
    return 10.f;
}
float Enemy::halfH() const {
    switch (type) {
        case EnemyType::LANDER: return LAND_H * 0.5f;
        case EnemyType::MUTANT: return MUTT_H * 0.5f;
        case EnemyType::BAITER: return BAIT_H * 0.5f;
    }
    return 8.f;
}

void Enemy::initLander(float wx, float y, float sm) {
    type      = EnemyType::LANDER;
    wpos      = {wx, y};
    vel       = {randf(-LAND_SPD * sm * 0.5f, LAND_SPD * sm * 0.5f), 0.f};
    lstate    = LanderState::WANDERING;
    humIdx    = -1;
    shotCD    = randf(1.f, LAND_SHOT_CD);
    wobble    = randf(0.f, 2.f * PI);
    stateCD   = randf(1.f, 3.f);
    speedMult = sm;
    alive     = true;
}

void Enemy::initMutant(float wx, float y, float sm) {
    type      = EnemyType::MUTANT;
    wpos      = {wx, y};
    vel       = {};
    humIdx    = -1;
    shotCD    = randf(0.5f, MUTT_SHOT_CD);
    wobble    = randf(0.f, 2.f * PI);
    stateCD   = 0.f;
    speedMult = sm;
    alive     = true;
}

void Enemy::initBaiter(float wx, float y, float sm) {
    type      = EnemyType::BAITER;
    wpos      = {wx, y};
    vel       = {};
    humIdx    = -1;
    shotCD    = randf(0.2f, BAIT_SHOT_CD);
    wobble    = randf(0.f, 2.f * PI);
    stateCD   = 0.f;
    speedMult = sm;
    alive     = true;
}

// -----------------------------------------------------------------------
void Enemy::updateLander(float dt, const Vector2& pWPos,
                         Humanoid* hums, int humCount, float groundY,
                         Vector2* shotOut, bool* wantShot)
{
    wobble += dt * 2.5f;

    if (lstate == LanderState::WANDERING) {
        // Slowly drift; periodically pick a humanoid target
        wpos.x = wrapX(wpos.x + vel.x * dt);
        wpos.y += sinf(wobble) * 20.f * dt;
        wpos.y = clampf(wpos.y, PLAY_TOP + 30.f, groundY - 40.f);

        stateCD -= dt;
        if (stateCD <= 0.f) {
            // Find nearest alive uncarried humanoid
            float bestDist = 1e9f;
            int   bestIdx  = -1;
            for (int i = 0; i < humCount; i++) {
                if (!hums[i].alive || hums[i].beingCarried) continue;
                float d = absf(wrapDX(wpos.x, hums[i].wx));
                if (d < bestDist) { bestDist = d; bestIdx = i; }
            }
            if (bestIdx >= 0) {
                humIdx = bestIdx;
                lstate = LanderState::DESCENDING;
            } else {
                stateCD = randf(1.5f, 3.f);
            }
        }
    }
    else if (lstate == LanderState::DESCENDING && humIdx >= 0) {
        Humanoid& h = hums[humIdx];
        if (!h.alive) { lstate = LanderState::WANDERING; humIdx = -1; stateCD = 1.f; return; }

        float dx = wrapDX(wpos.x, h.wx);
        float dy = h.y - wpos.y;
        float dist = sqrtf(dx*dx + dy*dy);

        if (dist < GRAB_DIST) {
            lstate = LanderState::GRABBING;
        } else {
            float spd = LAND_SPD * speedMult * 1.6f;
            wpos.x = wrapX(wpos.x + (dx / dist) * spd * dt);
            wpos.y += (dy / dist) * LAND_VY * speedMult * dt;
            wpos.y = clampf(wpos.y, PLAY_TOP + 20.f, groundY);
        }
    }
    else if (lstate == LanderState::GRABBING && humIdx >= 0) {
        Humanoid& h = hums[humIdx];
        if (!h.alive) { lstate = LanderState::WANDERING; humIdx = -1; stateCD = 1.f; return; }
        h.beingCarried = true;
        h.falling = false;
        lstate = LanderState::CARRYING;
    }
    else if (lstate == LanderState::CARRYING && humIdx >= 0) {
        Humanoid& h = hums[humIdx];
        // Move upward
        wpos.y -= LAND_VY * speedMult * 2.f * dt;
        wpos.x = wrapX(wpos.x + sinf(wobble) * 15.f * dt);
        // Keep humanoid attached below us
        h.wx = wpos.x;
        h.y  = wpos.y + LAND_H * 0.5f + HUM_H * 0.5f;

        if (wpos.y <= PLAY_TOP + 20.f) {
            lstate = LanderState::ASCENDED;
        }
    }

    // Shooting
    shotCD -= dt;
    if (shotCD <= 0.f) {
        float dx = wrapDX(wpos.x, pWPos.x);
        float dy = pWPos.y - wpos.y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist > 0.01f && dist < 600.f) {
            shotOut->x = wpos.x + (dx / dist) * SHOT_SPD;
            shotOut->y = wpos.y + (dy / dist) * SHOT_SPD;
            *wantShot = true;
        }
        shotCD = LAND_SHOT_CD + randf(-0.5f, 0.5f);
    }
}

void Enemy::updateMutant(float dt, const Vector2& pWPos,
                         Vector2* shotOut, bool* wantShot)
{
    wobble += dt * 3.f;
    float dx = wrapDX(wpos.x, pWPos.x);
    float dy = pWPos.y - wpos.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist > 0.01f) {
        float spd = MUTT_SPD * speedMult + 30.f * sinf(wobble * 0.7f);
        vel.x = (dx / dist) * spd;
        vel.y = (dy / dist) * spd;
    }
    wpos.x = wrapX(wpos.x + vel.x * dt);
    wpos.y = clampf(wpos.y + vel.y * dt, PLAY_TOP + 10.f, (float)SCREEN_H - 40.f);

    shotCD -= dt;
    if (shotCD <= 0.f && dist < 500.f) {
        shotOut->x = wpos.x + (dx / dist) * SHOT_SPD;
        shotOut->y = wpos.y + (dy / dist) * SHOT_SPD;
        *wantShot = true;
        shotCD = MUTT_SHOT_CD + randf(-0.3f, 0.3f);
    }
}

void Enemy::updateBaiter(float dt, const Vector2& pWPos,
                         Vector2* shotOut, bool* wantShot)
{
    wobble += dt * 4.f;
    float dx = wrapDX(wpos.x, pWPos.x);
    float dy = pWPos.y - wpos.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist > 0.01f) {
        vel.x = (dx / dist) * BAIT_SPD * speedMult;
        vel.y = (dy / dist) * BAIT_SPD * speedMult * 0.6f;
    }
    wpos.x = wrapX(wpos.x + vel.x * dt);
    wpos.y = clampf(wpos.y + vel.y * dt, PLAY_TOP + 10.f, (float)SCREEN_H - 40.f);

    shotCD -= dt;
    if (shotCD <= 0.f && dist < 700.f) {
        shotOut->x = wpos.x + (dx / dist) * SHOT_SPD * 1.2f;
        shotOut->y = wpos.y + (dy / dist) * SHOT_SPD * 1.2f;
        *wantShot = true;
        shotCD = BAIT_SHOT_CD + randf(-0.1f, 0.1f);
    }
}

void Enemy::update(float dt, const Vector2& pWPos,
                   Humanoid* hums, int humCount, float groundY,
                   Vector2* shotOut, bool* wantShot)
{
    *wantShot = false;
    if (!alive) return;

    switch (type) {
        case EnemyType::LANDER:
            updateLander(dt, pWPos, hums, humCount, groundY, shotOut, wantShot);
            break;
        case EnemyType::MUTANT:
            updateMutant(dt, pWPos, shotOut, wantShot);
            break;
        case EnemyType::BAITER:
            updateBaiter(dt, pWPos, shotOut, wantShot);
            break;
    }
}

// -----------------------------------------------------------------------
void Enemy::draw(float camX, float shakeX, float shakeY, const Sprites& spr) const {
    if (!alive) return;
    float sx = wsX(wpos.x, camX) + shakeX;
    float sy = wpos.y + shakeY;

    switch (type) {
    case EnemyType::LANDER: {
        float dw = 24.f, dh = 21.f;
        Rectangle src  = {0.f, 0.f, (float)spr.lander.width, (float)spr.lander.height};
        Rectangle dest = {sx - dw * 0.5f, sy - dh * 0.5f, dw, dh};
        DrawTexturePro(spr.lander, src, dest, {0.f, 0.f}, 0.f, WHITE);
        break;
    }
    case EnemyType::MUTANT: {
        float dw = 21.f, dh = 21.f;
        Rectangle src  = {0.f, 0.f, (float)spr.mutant.width, (float)spr.mutant.height};
        Rectangle dest = {sx - dw * 0.5f, sy - dh * 0.5f, dw, dh};
        DrawTexturePro(spr.mutant, src, dest, {0.f, 0.f}, 0.f, WHITE);
        break;
    }
    case EnemyType::BAITER: {
        float dw = 24.f, dh = 12.f;
        Rectangle src  = {0.f, 0.f, (float)spr.baiter.width, (float)spr.baiter.height};
        Rectangle dest = {sx - dw * 0.5f, sy - dh * 0.5f, dw, dh};
        DrawTexturePro(spr.baiter, src, dest, {0.f, 0.f}, 0.f, WHITE);
        // Rotating scanner line drawn on top
        float scan = fmodf(wobble, 2.f * PI);
        float hw = halfW();
        float hh = halfH();
        DrawLine((int)sx, (int)sy,
                 (int)(sx + cosf(scan) * hw),
                 (int)(sy + sinf(scan) * hh), {255, 80, 80, 255});
        break;
    }
    }
}
