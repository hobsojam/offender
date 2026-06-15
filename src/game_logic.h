#pragma once

struct ScoreState {
    int score;
    int hiScore;
    int lives;
};

struct HumanoidReleaseState {
    float wx;
    float y;
    float groundY;
    float vy;
    bool falling;
    bool beingCarried;
};

inline int AddScore(ScoreState& state, int points, int extraLifeEvery) {
    if (points <= 0 || extraLifeEvery <= 0) return 0;

    int oldScore = state.score;
    state.score += points;
    if (state.score > state.hiScore) state.hiScore = state.score;

    int oldMilestone = oldScore / extraLifeEvery;
    int newMilestone = state.score / extraLifeEvery;
    int extraLives = newMilestone - oldMilestone;
    if (extraLives > 0) {
        state.lives += extraLives;
        return extraLives;
    }

    return 0;
}

inline void ReleaseHumanoid(HumanoidReleaseState& state, float worldX,
                            float groundY, bool falling) {
    state.wx = worldX;
    state.groundY = groundY;
    state.beingCarried = false;
    state.falling = falling;
    state.vy = 0.f;

    if (!falling && state.y > state.groundY)
        state.y = state.groundY;
}
