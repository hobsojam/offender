#pragma once
#include <string>

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
    if (int extraLives = newMilestone - oldMilestone; extraLives > 0) {
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

inline std::string JoinPath(const std::string& dir, const std::string& name) {
    if (dir.empty()) return name;

    if (char last = dir[dir.size() - 1]; last == '/' || last == '\\')
        return dir + name;

#ifdef _WIN32
    return dir + "\\" + name;
#else
    return dir + "/" + name;
#endif
}

inline std::string BuildHiScorePath(const std::string& appData,
                                    const std::string& xdgDataHome,
                                    const std::string& home,
                                    const std::string& fallbackDir) {
#ifdef _WIN32
    if (!appData.empty())
        return JoinPath(JoinPath(appData, "Offender"), "hiscore.dat");
#else
    (void)appData;

    if (!xdgDataHome.empty())
        return JoinPath(JoinPath(xdgDataHome, "offender"), "hiscore.dat");

    if (!home.empty())
        return JoinPath(JoinPath(JoinPath(home, ".local"), "share"), "offender/hiscore.dat");
#endif

    return JoinPath(fallbackDir, "hiscore.dat");
}
