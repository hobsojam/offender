#include "game.h"
#include "game_logic.h"
#include "radar.h"
#include "hud.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <filesystem>
#include <string>
#include <system_error>

static float randf(float lo, float hi) {
    return lo + (float)rand() / (float)RAND_MAX * (hi - lo);
}

static std::string envOrEmpty(const char* name) {
    const char* value = getenv(name);
    return value ? value : "";
}

static std::string hiScorePath() {
    return BuildHiScorePath(envOrEmpty("APPDATA"),
                            envOrEmpty("XDG_DATA_HOME"),
                            envOrEmpty("HOME"),
                            ".");
}

static bool ensureParentDirectory(const std::string& path) {
    std::filesystem::path fsPath(path);
    if (!fsPath.has_parent_path()) return true;

    std::error_code ec;
    return std::filesystem::create_directories(fsPath.parent_path(), ec) ||
           std::filesystem::exists(fsPath.parent_path(), ec);
}

static FILE* openHiScoreFile(const char* mode) {
    std::string path = hiScorePath();
    if (mode[0] != 'w' || ensureParentDirectory(path)) {
        if (FILE* f = fopen(path.c_str(), mode))
            return f;
    }

    std::string fallback = JoinPath(".", "hiscore.dat");
    if (fallback != path)
        return fopen(fallback.c_str(), mode);

    return nullptr;
}

// -----------------------------------------------------------------------
void Game::init() {
    const char* seedEnv = getenv("OFFENDER_SEED");
    unsigned seed = seedEnv ? (unsigned)strtoul(seedEnv, nullptr, 10)
                            : (unsigned)std::chrono::system_clock::now()
                                  .time_since_epoch()
                                  .count();
    srand(seed);
    world.init();
    audio.init();
    sprites.load();
    memset(particles, 0, sizeof(particles));
    memset(lasers,    0, sizeof(lasers));
    memset(shots,     0, sizeof(shots));
    state   = GameState::TITLE;
    hiScore = 0;

    FILE* f = openHiScoreFile("r");
    if (f) { fscanf(f, "%d", &hiScore); fclose(f); }
}

void Game::shutdown() {
    FILE* f = openHiScoreFile("w");
    if (f) { fprintf(f, "%d\n", hiScore); fclose(f); }

    sprites.unload();
    audio.shutdown();
}

// -----------------------------------------------------------------------
void Game::startNewGame() {
    score      = 0;
    lives      = START_LIVES;
    bombs      = START_BOMBS;
    wave       = 1;
    memset(lasers,    0, sizeof(lasers));
    memset(shots,     0, sizeof(shots));
    memset(particles, 0, sizeof(particles));
    world.init();
    startWave();
    audio.startBGM();
}

void Game::startWave() {
    baitTimer       = 0.f;
    stateTimer      = 0.f;
    planetDestroyed = false;
    enemyCount      = 0;
    humCount   = 0;
    memset(enemies, 0, sizeof(enemies));
    memset(hums,    0, sizeof(hums));

    // Place humanoids on terrain at even intervals with some jitter
    humCount = HUM_COUNT;
    float spacing = WORLD_W / HUM_COUNT;
    for (int i = 0; i < humCount; i++) {
        float wx = randf(i * spacing, (i + 1) * spacing);
        float gy = world.terrainY(wx);
        hums[i].init(wx, gy);
    }

    spawnWaveEnemies();

    // Respawn player at centre-ish of world
    float startX = WORLD_W * 0.5f;
    float gy     = world.terrainY(startX);
    player.init(startX, gy);
    world.camX = wrapX(startX - SCREEN_W * 0.5f);

    state = GameState::PLAYING;
}

void Game::spawnWaveEnemies() {
    int landerCount = 3 + wave;
    if (landerCount > MAX_ENEMIES - 4) landerCount = MAX_ENEMIES - 4;

    float spacing = WORLD_W / landerCount;
    for (int i = 0; i < landerCount && enemyCount < MAX_ENEMIES; i++) {
        float wx = randf(i * spacing, (i + 1) * spacing);
        float y  = randf(PLAY_TOP + 60.f, PLAY_TOP + 200.f);
        enemies[enemyCount++].initLander(wx, y);
    }
}

void Game::startRespawn() {
    lives--;
    if (lives <= 0) {
        if (score > hiScore) hiScore = score;
        audio.stopBGM();
        state      = GameState::GAME_OVER;
        stateTimer = 0.f;
        return;
    }
    // Respawn after pause
    float startX = wrapX(player.pos.x + randf(-300.f, 300.f));
    float gy     = world.terrainY(startX);
    player.init(startX, gy);
    state      = GameState::PLAYER_DEAD;
    stateTimer = DEATH_PAUSE;
    audio.playDie();
}

// -----------------------------------------------------------------------
void Game::update(float dt) {
    if (dt > 0.05f) dt = 0.05f;  // cap timestep

    switch (state) {
    case GameState::TITLE:
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            startNewGame();
        break;

    case GameState::PLAYING:
        updatePlaying(dt);
        break;

    case GameState::PLAYER_DEAD:
        updatePlayerDead(dt);
        break;

    case GameState::WAVE_CLEAR:
        updateWaveClear(dt);
        break;

    case GameState::PAUSED:
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            state = GameState::PLAYING;
        break;

    case GameState::GAME_OVER:
        stateTimer += dt;
        if (stateTimer > 1.5f &&
            (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)))
            state = GameState::TITLE;
        break;
    }

    updateParticles(dt);
    updateShake(dt);

    if (state == GameState::PLAYING    || state == GameState::PAUSED ||
        state == GameState::PLAYER_DEAD || state == GameState::WAVE_CLEAR)
        audio.updateBGM();
}

void Game::updatePlaying(float dt) {
    player.handleInput(dt);

    // Fire laser
    if ((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_LEFT_CONTROL)) && player.canFire())
        fireLaser();

    // Smart bomb
    if ((IsKeyPressed(KEY_B) || IsKeyPressed(KEY_Z)) && bombs > 0)
        smartBomb();

    // Hyperspace
    if (IsKeyPressed(KEY_H) || IsKeyPressed(KEY_LEFT_SHIFT))
        doHyperspace();

    // Update player
    float gy = world.terrainY(player.pos.x);
    player.update(dt, gy);
    world.updateCamera(player.pos.x, player.facingRight, dt);

    // Update lasers
    for (int i = 0; i < MAX_LASERS; i++) {
        Laser& l = lasers[i];
        if (!l.active) continue;
        l.wx = wrapX(l.wx + l.vx * dt);
        l.life -= dt;
        if (l.life <= 0.f) l.active = false;
    }

    // Update shots
    for (int i = 0; i < MAX_SHOTS; i++) {
        Shot& s = shots[i];
        if (!s.active) continue;
        s.wpos.x = wrapX(s.wpos.x + s.vel.x * dt);
        s.wpos.y += s.vel.y * dt;
        s.life -= dt;
        if (s.life <= 0.f || s.wpos.y < PLAY_TOP || s.wpos.y > SCREEN_H)
            s.active = false;
    }

    // Update enemies + collect new shots
    Vector2 pWPos = { player.pos.x, player.pos.y };
    for (int i = 0; i < enemyCount; i++) {
        Enemy& e = enemies[i];
        if (!e.alive) continue;

        float terrainYAtE = world.terrainY(e.wpos.x);
        Vector2 shotVel = {};
        bool wantShot = false;

        e.update(dt, pWPos, hums, humCount, terrainYAtE, &shotVel, &wantShot);

        // Check if lander reached top while carrying → become mutant
        if (e.type == EnemyType::LANDER &&
            e.lstate == LanderState::ASCENDED)
        {
            if (e.humIdx >= 0 && e.humIdx < humCount) {
                hums[e.humIdx].beingCarried = false;
                hums[e.humIdx].alive = false;  // abducted!
            }
            e.initMutant(e.wpos.x, e.wpos.y);
            audio.playAbduct();
        }

        // Spawn shot if enemy requested it
        if (wantShot) {
            for (int j = 0; j < MAX_SHOTS; j++) {
                if (!shots[j].active) {
                    shots[j].wpos = e.wpos;
                    float dx = shotVel.x - e.wpos.x;
                    float dy = shotVel.y - e.wpos.y;
                    float dist = sqrtf(dx*dx + dy*dy);
                    if (dist > 0.01f) {
                        shots[j].vel = {dx / dist * SHOT_SPD, dy / dist * SHOT_SPD};
                    } else {
                        shots[j].vel = {SHOT_SPD, 0.f};
                    }
                    shots[j].life   = SHOT_LIFE;
                    shots[j].active = true;
                    break;
                }
            }
        }
    }

    // Update humanoids
    for (int i = 0; i < humCount; i++)
        hums[i].update(dt);

    // Baiter spawning
    baitTimer += dt;
    if (baitTimer >= BAIT_SPAWN_TIME) {
        baitTimer -= BAIT_SPAWN_TIME * 0.5f;  // spawn periodically
        if (enemyCount < MAX_ENEMIES) {
            float wx = wrapX(player.pos.x + randf(300.f, 500.f) * (randf(0,1) > 0.5f ? 1 : -1));
            enemies[enemyCount++].initBaiter(wx, player.pos.y + randf(-80.f, 80.f));
        }
    }

    checkCollisions();

    // checkCollisions() may have transitioned state (player died, game over).
    // Do not override that transition with wave-clear or planet-destroyed logic.
    if (state != GameState::PLAYING) return;

    // Check wave clear
    if (allEnemiesDead()) {
        for (int i = 0; i < humCount; i++)
            if (hums[i].alive) addScore(SC_HUM_BONUS);
        state      = GameState::WAVE_CLEAR;
        stateTimer = WAVE_CLEAR_PAUSE;
        return;
    }

    // If all humanoids are dead, convert any remaining landers to mutants.
    // planetDestroyed flag prevents this from running every frame.
    if (!planetDestroyed && liveHumCount() == 0) {
        planetDestroyed = true;
        for (int i = 0; i < enemyCount; i++) {
            Enemy& e = enemies[i];
            if (e.alive && e.type == EnemyType::LANDER)
                e.initMutant(e.wpos.x, e.wpos.y);
        }
    }
}

void Game::updatePlayerDead(float dt) {
    stateTimer -= dt;
    world.updateCamera(player.pos.x, player.facingRight, dt);
    if (stateTimer <= 0.f) {
        player.alive = true;
        player.invTimer = INVINCIBLE_TIME;
        state = GameState::PLAYING;
    }
}

void Game::updateWaveClear(float dt) {
    stateTimer -= dt;
    if (stateTimer <= 0.f) {
        wave++;
        startWave();
    }
}

void Game::togglePause() {
    if (state == GameState::PLAYING)     state = GameState::PAUSED;
    else if (state == GameState::PAUSED) state = GameState::PLAYING;
}

// -----------------------------------------------------------------------
void Game::fireLaser() {
    for (int i = 0; i < MAX_LASERS; i++) {
        if (!lasers[i].active) {
            lasers[i].wx     = player.pos.x;
            lasers[i].y      = player.pos.y;
            lasers[i].vx     = player.facingRight ? LASER_SPEED : -LASER_SPEED;
            lasers[i].life   = LASER_LIFE;
            lasers[i].active = true;
            player.laserCD   = LASER_CD;
            audio.playFire();
            return;
        }
    }
}

void Game::smartBomb() {
    if (bombs <= 0) return;
    bombs--;
    audio.playBomb();

    // Destroy all on-screen enemies
    for (int i = 0; i < enemyCount; i++) {
        Enemy& e = enemies[i];
        if (!e.alive) continue;
        float sx = wsX(e.wpos.x, world.camX);
        if (sx >= -10.f && sx <= SCREEN_W + 10.f) {
            if (e.type == EnemyType::LANDER &&
                (e.lstate == LanderState::GRABBING || e.lstate == LanderState::CARRYING) &&
                e.humIdx >= 0 && e.humIdx < humCount)
            {
                releaseHumanoid(e.humIdx, e.wpos.x, true);
            }
            spawnExplosion(e.wpos, {255, 200, 50, 255}, 16, 140.f);
            int pts = (e.type == EnemyType::LANDER) ? SC_LANDER :
                      (e.type == EnemyType::MUTANT) ? SC_MUTANT : SC_BAITER;
            addScore(pts);
            e.alive = false;
        }
    }

    // Screen flash
    shakeTimer = SHAKE_DURATION;
    shakeX = shakeY = 0.f;
}

void Game::doHyperspace() {
    audio.playHyper();
    float newX = randf(0.f, WORLD_W);
    float newY = randf(PLAY_TOP + 60.f, P_MAX_Y - 20.f);
    player.pos.x = newX;
    player.pos.y = newY;
    player.vel   = {0.f, 0.f};
    player.invTimer = INVINCIBLE_TIME * 0.5f;
    spawnExplosion({newX, newY}, {100, 200, 255, 255}, 12, 90.f);
}

void Game::addScore(int points) {
    ScoreState scoreState = {score, hiScore, lives};
    if (int extraLives = AddScore(scoreState, points, EXTRA_LIFE_EVERY); extraLives > 0) {
        score = scoreState.score;
        hiScore = scoreState.hiScore;
        lives = scoreState.lives;
        audio.playExtraLife();
        return;
    }

    score = scoreState.score;
    hiScore = scoreState.hiScore;
    lives = scoreState.lives;
}

void Game::releaseHumanoid(int humIdx, float worldX, bool falling) {
    if (humIdx < 0 || humIdx >= humCount) return;

    Humanoid& h = hums[humIdx];
    float releaseX = wrapX(worldX);
    HumanoidReleaseState releaseState = {
        h.wx,
        h.y,
        h.groundY,
        h.vy,
        h.falling,
        h.beingCarried
    };
    ReleaseHumanoid(releaseState, releaseX, world.terrainY(releaseX), falling);

    h.wx = releaseState.wx;
    h.y = releaseState.y;
    h.groundY = releaseState.groundY;
    h.vy = releaseState.vy;
    h.falling = releaseState.falling;
    h.beingCarried = releaseState.beingCarried;
}

// -----------------------------------------------------------------------
bool Game::hitPlayer(float ewx, float esy, float ew, float eh) {
    if (!player.alive || player.invincible()) return false;
    float dx = absf(wrapDX(player.pos.x, ewx));
    float dy = absf(player.pos.y - esy);
    return dx < (P_W + ew) * 0.5f && dy < (P_H + eh) * 0.5f;
}

void Game::checkCollisions() {
    // Lasers vs enemies
    for (int li = 0; li < MAX_LASERS; li++) {
        Laser& la = lasers[li];
        if (!la.active) continue;
        for (int ei = 0; ei < enemyCount; ei++) {
            Enemy& e = enemies[ei];
            if (!e.alive) continue;
            float dx = absf(wrapDX(la.wx, e.wpos.x));
            float dy = absf(la.y - e.wpos.y);
            if (dx < (LASER_W + e.halfW() * 2.f) * 0.5f &&
                dy < (LASER_H + e.halfH() * 2.f) * 0.5f)
            {
                la.active = false;
                e.alive   = false;

                // Release humanoid only if this lander was actively carrying it.
                // DESCENDING landers have a humIdx target but haven't grabbed yet.
                if (e.type == EnemyType::LANDER &&
                    (e.lstate == LanderState::GRABBING || e.lstate == LanderState::CARRYING) &&
                    e.humIdx >= 0 && e.humIdx < humCount)
                {
                    releaseHumanoid(e.humIdx, e.wpos.x, true);
                }

                int pts = (e.type == EnemyType::LANDER) ? SC_LANDER :
                          (e.type == EnemyType::MUTANT)  ? SC_MUTANT  : SC_BAITER;
                addScore(pts);

                spawnExplosion(e.wpos,
                    e.type == EnemyType::MUTANT ? Color{200,80,255,255} :
                    e.type == EnemyType::BAITER ? Color{255,220,0,255}  :
                                                   Color{255,120,60,255});
                audio.playExplode();
                shakeTimer = SHAKE_DURATION * 0.5f;
                break;
            }
        }
    }

    // Player vs enemies
    for (int ei = 0; ei < enemyCount; ei++) {
        Enemy& e = enemies[ei];
        if (!e.alive) continue;
        if (hitPlayer(e.wpos.x, e.wpos.y, e.halfW()*2.f, e.halfH()*2.f)) {
            spawnExplosion(player.pos, {255, 200, 80, 255}, 20, 130.f);
            player.alive = false;
            shakeTimer   = SHAKE_DURATION;
            startRespawn();
            return;
        }
    }

    // Player vs shots
    for (int i = 0; i < MAX_SHOTS; i++) {
        Shot& s = shots[i];
        if (!s.active) continue;
        if (hitPlayer(s.wpos.x, s.wpos.y, 4.f, 4.f)) {
            s.active = false;
            spawnExplosion(player.pos, {255, 200, 80, 255}, 20, 130.f);
            player.alive = false;
            shakeTimer   = SHAKE_DURATION;
            startRespawn();
            return;
        }
    }

    // Player catches falling humanoids.
    // Immediately ground them at the terrain below the catch point to prevent
    // the catch from re-firing every frame while the player is nearby.
    for (int i = 0; i < humCount; i++) {
        Humanoid& h = hums[i];
        if (!h.alive || !h.falling) continue;
        float dx = absf(wrapDX(player.pos.x, h.wx));
        float dy = absf(player.pos.y - h.y);
        if (dx < CATCH_DIST && dy < CATCH_DIST) {
            float terr = world.terrainY(player.pos.x);
            h.wx          = player.pos.x;
            h.groundY     = terr;
            h.y           = terr;
            h.vy          = 0.f;
            h.falling     = false;
            h.beingCarried = false;
            addScore(SC_CATCH);
            audio.playRescue();
        }
    }
}

// -----------------------------------------------------------------------
void Game::spawnExplosion(Vector2 pos, Color col, int count, float spd) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!particles[j].active) {
                float angle = randf(0.f, 2.f * PI);
                float speed = randf(spd * 0.3f, spd);
                particles[j].pos     = pos;
                particles[j].vel     = { cosf(angle) * speed, sinf(angle) * speed };
                particles[j].life    = randf(0.3f, 0.8f);
                particles[j].maxLife = particles[j].life;
                particles[j].size    = randf(2.f, 5.f);
                particles[j].col     = col;
                particles[j].active  = true;
                break;
            }
        }
    }
}

void Game::updateParticles(float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle& p = particles[i];
        if (!p.active) continue;
        p.pos.x = wrapX(p.pos.x + p.vel.x * dt);
        p.pos.y += p.vel.y * dt;
        p.vel.x *= (1.f - dt * 1.5f);
        p.vel.y *= (1.f - dt * 1.5f);
        p.life -= dt;
        if (p.life <= 0.f) { p.active = false; }
    }
}

void Game::updateShake(float dt) {
    if (shakeTimer > 0.f) {
        shakeTimer -= dt;
        float mag = (shakeTimer / SHAKE_DURATION) * SHAKE_AMT;
        shakeX = randf(-mag, mag);
        shakeY = randf(-mag, mag);
    } else {
        shakeX = shakeY = 0.f;
    }
}

// -----------------------------------------------------------------------
int  Game::liveHumCount() const {
    int n = 0;
    for (int i = 0; i < humCount; i++)
        if (hums[i].alive) n++;
    return n;
}

bool Game::allEnemiesDead() const {
    if (enemyCount == 0) return false;
    for (int i = 0; i < enemyCount; i++)
        if (enemies[i].alive) return false;
    return true;
}

// -----------------------------------------------------------------------
void Game::draw() {
    switch (state) {
    case GameState::TITLE:      drawTitle();     break;
    case GameState::GAME_OVER:  drawGameOver();  break;
    case GameState::WAVE_CLEAR: drawWaveClear(); break;
    case GameState::PAUSED:
        drawPlaying();
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 140});
        {
            const char* msg = "PAUSED";
            int tw = MeasureText(msg, 60);
            DrawText(msg, (SCREEN_W - tw) / 2, SCREEN_H / 2 - 30, 60, {255, 220, 0, 255});
            const char* sub = "P or ENTER to resume";
            int sw = MeasureText(sub, 22);
            DrawText(sub, (SCREEN_W - sw) / 2, SCREEN_H / 2 + 44, 22, WHITE);
        }
        break;
    default:
        drawPlaying();
        break;
    }
}

void Game::drawPlaying() const {
    // World (stars + terrain)
    world.draw();

    // Humanoids
    for (int i = 0; i < humCount; i++)
        hums[i].draw(world.camX, shakeX, shakeY, sprites);

    // Shots
    for (int i = 0; i < MAX_SHOTS; i++) {
        const Shot& s = shots[i];
        if (!s.active) continue;
        float sx = wsX(s.wpos.x, world.camX) + shakeX;
        float sy = s.wpos.y + shakeY;
        DrawCircle((int)sx, (int)sy, 3.f, {255, 100, 0, 255});
    }

    // Enemies
    for (int i = 0; i < enemyCount; i++)
        enemies[i].draw(world.camX, shakeX, shakeY, sprites);

    // Lasers
    for (int i = 0; i < MAX_LASERS; i++) {
        const Laser& l = lasers[i];
        if (!l.active) continue;
        float sx = wsX(l.wx, world.camX) + shakeX;
        float sy = l.y + shakeY;
        DrawRectangle((int)(sx - LASER_W * 0.5f), (int)(sy - LASER_H * 0.5f),
                      (int)LASER_W, (int)LASER_H, {0, 255, 220, 255});
    }

    // Player
    player.draw(world.camX, shakeX, shakeY, sprites);

    // Particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        const Particle& p = particles[i];
        if (!p.active) continue;
        float alpha = p.life / p.maxLife;
        Color c = p.col;
        c.a = (unsigned char)(alpha * 255.f);
        float sx = wsX(p.pos.x, world.camX) + shakeX;
        float sy = p.pos.y + shakeY;
        DrawCircle((int)sx, (int)sy, p.size * alpha, c);
    }

    // Radar + HUD (always on top, no shake)
    DrawRadar(world.camX, player.pos.x, player.pos.y,
              enemies, enemyCount, hums, humCount, world.terrain);
    DrawHUD(score, hiScore, lives, wave, bombs);

    // Dead player message
    if (state == GameState::PLAYER_DEAD) {
        const char* msg = "SHIP DESTROYED";
        int tw = MeasureText(msg, 30);
        DrawText(msg, (SCREEN_W - tw) / 2, SCREEN_H / 2 - 15, 30, {255, 80, 80, 255});
    }

    // Controls reminder bottom-left (first 5 sec of wave 1)
    if (wave == 1 && baitTimer < 5.f) {
        DrawText("ARROWS/WASD: Move   SPACE: Fire   B/Z: Bomb   H: Hyperspace",
                 8, SCREEN_H - 18, 10, {120, 120, 120, 200});
    }
}

void Game::drawTitle() const {
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, BLACK);

    // Starfield (use world stars even on title)
    for (int i = 0; i < NUM_STARS; i++) {
        int sx = (int)(world.stars[i].x / WORLD_W * SCREEN_W);
        int sy = (int)world.stars[i].y;
        DrawPixel(sx, sy, {140, 140, 180, 255});
    }

    const char* title = "OFFENDER";
    int tw = MeasureText(title, 80);
    DrawText(title, (SCREEN_W - tw) / 2, 160, 80, {0, 255, 0, 255});

    const char* sub = "A DEFENDER CLONE";
    int sw = MeasureText(sub, 24);
    DrawText(sub, (SCREEN_W - sw) / 2, 260, 24, {0, 180, 0, 255});

    const char* ctrl = "ARROWS / WASD : Move     SPACE : Fire\n"
                       "B / Z : Smart Bomb       H : Hyperspace\n"
                       "P : Pause                ESC : Quit";
    DrawText(ctrl, SCREEN_W / 2 - 220, 360, 20, {150, 200, 255, 255});

    // Blink prompt
    if ((int)(GetTime() * 2.0) % 2 == 0) {
        const char* prompt = "PRESS ENTER OR SPACE TO START";
        int pw = MeasureText(prompt, 22);
        DrawText(prompt, (SCREEN_W - pw) / 2, 500, 22, {255, 220, 0, 255});
    }

    if (hiScore > 0) {
        char buf[32];
        snprintf(buf, sizeof(buf), "HI SCORE  %07d", hiScore);
        int hw = MeasureText(buf, 20);
        DrawText(buf, (SCREEN_W - hw) / 2, 580, 20, {255, 200, 0, 255});
    }
}

void Game::drawGameOver() const {
    drawPlaying();  // keep game visible behind overlay

    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 160});

    const char* msg = "GAME OVER";
    int tw = MeasureText(msg, 72);
    DrawText(msg, (SCREEN_W - tw) / 2, SCREEN_H / 2 - 60, 72, {255, 50, 50, 255});

    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE  %07d", score);
    int sw = MeasureText(buf, 28);
    DrawText(buf, (SCREEN_W - sw) / 2, SCREEN_H / 2 + 30, 28, WHITE);

    if (stateTimer > 1.5f && (int)(GetTime() * 2.0) % 2 == 0) {
        const char* prompt = "PRESS ENTER OR SPACE";
        int pw = MeasureText(prompt, 20);
        DrawText(prompt, (SCREEN_W - pw) / 2, SCREEN_H / 2 + 80, 20, {255, 220, 0, 255});
    }
}

void Game::drawWaveClear() const {
    drawPlaying();
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 120});

    char buf[48];
    snprintf(buf, sizeof(buf), "WAVE %d CLEAR", wave);
    int tw = MeasureText(buf, 52);
    DrawText(buf, (SCREEN_W - tw) / 2, SCREEN_H / 2 - 40, 52, {0, 255, 150, 255});

    int bonus = liveHumCount() * SC_HUM_BONUS;
    if (bonus > 0) {
        snprintf(buf, sizeof(buf), "HUMANOID BONUS  +%d", bonus);
        int bw = MeasureText(buf, 22);
        DrawText(buf, (SCREEN_W - bw) / 2, SCREEN_H / 2 + 30, 22, {255, 220, 100, 255});
    }
}
