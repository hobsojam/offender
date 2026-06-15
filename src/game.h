#pragma once
#include "world.h"
#include "player.h"
#include "projectile.h"
#include "enemies.h"
#include "humanoid.h"
#include "audio.h"
#include "sprites.h"

enum class GameState { TITLE, PLAYING, PAUSED, PLAYER_DEAD, WAVE_CLEAR, GAME_OVER };

class Game {
public:
    void      init();
    void      update(float dt);
    void      draw();
    void      shutdown();
    GameState getState()    const { return state; }
    void      togglePause();

private:
    GameState  state        = GameState::TITLE;
    int        score        = 0;
    int        hiScore      = 0;
    int        lives        = START_LIVES;
    int        wave         = 1;
    int        bombs        = START_BOMBS;
    int        lastElScore  = 0;
    float      stateTimer   = 0.f;
    float      baitTimer        = 0.f;
    bool       planetDestroyed  = false;
    float      shakeTimer   = 0.f;
    float      shakeX       = 0.f;
    float      shakeY       = 0.f;

    World      world;
    Player     player;

    Laser      lasers[MAX_LASERS]       = {};
    Shot       shots[MAX_SHOTS]         = {};
    Enemy      enemies[MAX_ENEMIES]     = {};
    int        enemyCount               = 0;
    Humanoid   hums[HUM_COUNT]          = {};
    int        humCount                 = 0;
    Particle   particles[MAX_PARTICLES] = {};

    AudioFX    audio;
    Sprites    sprites;

    void startNewGame();
    void startWave();
    void spawnWaveEnemies();
    void startRespawn();

    void updatePlaying(float dt);
    void updatePlayerDead(float dt);
    void updateWaveClear(float dt);

    void fireLaser();
    void smartBomb();
    void doHyperspace();

    void checkCollisions();
    bool hitPlayer(float ewx, float esy, float ew, float eh);

    void spawnExplosion(Vector2 pos, Color col, int count = 14, float spd = 110.f);
    void updateParticles(float dt);
    void updateShake(float dt);

    void drawPlaying()   const;
    void drawTitle()     const;
    void drawGameOver()  const;
    void drawWaveClear() const;

    int  liveHumCount()  const;
    bool allEnemiesDead() const;
};
