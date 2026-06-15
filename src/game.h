#pragma once
#include "world.h"
#include "player.h"
#include "projectile.h"
#include "enemies.h"
#include "humanoid.h"
#include "audio.h"
#include "sprites.h"

enum class GameState { TITLE, PLAYING, PAUSED, PLAYER_DEAD, WAVE_CLEAR, GAME_OVER };

struct ScorePopup {
    float wx, y;        // world X, screen Y of spawn point
    float vy;           // drifts upward
    float life, maxLife;
    char  text[12];
    Color col;
    bool  active;
};

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
    float      stateTimer   = 0.f;
    float      baitTimer        = 0.f;
    float      hintTimer        = 0.f;
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
    int        waveSurvivors            = 0;
    Particle    particles[MAX_PARTICLES] = {};
    int         particleNext             = 0;
    ScorePopup  popups[MAX_POPUPS]      = {};

    AudioFX    audio;
    Sprites    sprites;

    void startNewGame();
    void startWave(int prevSurvivors = -1);
    void spawnWaveEnemies();
    void startRespawn();

    void updatePlaying(float dt);
    void updatePlayerDead(float dt);
    void updateWaveClear(float dt);

    void fireLaser();
    void smartBomb();
    void doHyperspace();
    void addScore(int points);
    void loseScore(int points);
    void releaseHumanoid(int humIdx, float worldX, bool falling);

    void checkCollisions();
    void compactEnemies();
    bool hitPlayer(float ewx, float esy, float ew, float eh);

    void spawnExplosion(Vector2 pos, Color col, int count = 14, float spd = 110.f);
    void spawnPopup(float wx, float y, int points, Color col);
    void updateParticles(float dt);
    void updatePopups(float dt);
    void updateShake(float dt);

    void drawPlaying()   const;
    void drawTitle()     const;
    void drawGameOver()  const;
    void drawWaveClear() const;

    int  liveHumCount()  const;
    bool allEnemiesDead() const;
    float waveSpeedMult() const { return std::min(1.f + (wave - 1) * 0.1f, 2.f); }
};
