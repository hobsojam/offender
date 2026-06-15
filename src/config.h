#pragma once

static constexpr int   SCREEN_W         = 1280;
static constexpr int   SCREEN_H         = 720;
static constexpr float RADAR_H          = 48.f;
static constexpr float PLAY_TOP         = RADAR_H;
static constexpr float PLAY_H           = (float)SCREEN_H - RADAR_H;

static constexpr float WORLD_W          = 6400.f;
static constexpr int   TERR_NODES       = 200;
static constexpr float TERR_SEG         = WORLD_W / TERR_NODES;

static constexpr float GROUND_BASE      = 620.f;
static constexpr float GROUND_AMP       = 38.f;

static constexpr int   NUM_STARS        = 180;

// Player
static constexpr float P_MAX_VX         = 460.f;
static constexpr float P_ACCEL          = 760.f;
static constexpr float P_FRICTION       = 300.f;
static constexpr float P_VY             = 270.f;
static constexpr float P_W              = 28.f;
static constexpr float P_H              = 12.f;
static constexpr float P_MIN_Y          = PLAY_TOP + 20.f;
static constexpr float P_MAX_Y          = (float)SCREEN_H - 20.f;

// Laser
static constexpr float LASER_SPEED      = 1840.f;
static constexpr float LASER_LIFE       = 0.65f;
static constexpr int   MAX_LASERS       = 4;
static constexpr float LASER_W          = 28.f;
static constexpr float LASER_H          = 3.f;
static constexpr float LASER_CD         = 0.14f;

// Enemies
static constexpr float LAND_SPD         = 58.f;
static constexpr float LAND_VY          = 50.f;
static constexpr float LAND_W           = 20.f;
static constexpr float LAND_H           = 16.f;
static constexpr float LAND_SHOT_CD     = 3.4f;

static constexpr float MUTT_SPD         = 200.f;
static constexpr float MUTT_W           = 18.f;
static constexpr float MUTT_H           = 14.f;
static constexpr float MUTT_SHOT_CD     = 1.5f;

static constexpr float BAIT_SPD         = 240.f;
static constexpr float BAIT_W           = 16.f;
static constexpr float BAIT_H           = 10.f;
static constexpr float BAIT_SHOT_CD     = 0.7f;
static constexpr float BAIT_SPAWN_TIME  = 20.f;

static constexpr float SHOT_SPD         = 230.f;
static constexpr float SHOT_LIFE        = 2.2f;

// Humanoids
static constexpr int   HUM_COUNT        = 10;
static constexpr float HUM_W            = 8.f;
static constexpr float HUM_H            = 14.f;
static constexpr float HUM_FALL_SPD     = 130.f;
static constexpr float GRAB_DIST        = 14.f;
static constexpr float CATCH_DIST       = 20.f;

// Scoring
static constexpr int   SC_LANDER        = 150;
static constexpr int   SC_MUTANT        = 150;
static constexpr int   SC_BAITER        = 200;
static constexpr int   SC_RESCUE        = 500;
static constexpr int   SC_CATCH         = 250;
static constexpr int   SC_HUM_BONUS     = 100;
static constexpr int   EXTRA_LIFE_EVERY = 10000;

// Game
static constexpr int   START_LIVES      = 3;
static constexpr int   START_BOMBS      = 3;
static constexpr int   MAX_BOMBS        = 3;
static constexpr float DEATH_PAUSE      = 2.2f;
static constexpr float WAVE_CLEAR_PAUSE = 2.8f;
static constexpr float INVINCIBLE_TIME  = 3.0f;
static constexpr float SHAKE_DURATION  = 0.45f;
static constexpr float SHAKE_AMT       = 6.f;

static constexpr int   MAX_PARTICLES    = 256;
static constexpr int   MAX_ENEMIES      = 80;
static constexpr int   MAX_SHOTS        = 32;
