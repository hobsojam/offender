#include "game_logic.h"

#include <cstdlib>
#include <iostream>
#include <string>

static void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

static void testAddScoreUpdatesScoreAndHiScore() {
    ScoreState state = {900, 1000, 3};

    int extraLives = AddScore(state, 200, 10000);

    require(extraLives == 0, "unexpected extra life before milestone");
    require(state.score == 1100, "score should increase");
    require(state.hiScore == 1100, "hi-score should track new high score");
    require(state.lives == 3, "lives should not change before milestone");
}

static void testAddScoreAwardsEachMilestoneCrossed() {
    ScoreState state = {9500, 9500, 3};

    int extraLives = AddScore(state, 21000, 10000);

    require(extraLives == 3, "should award one life per crossed milestone");
    require(state.score == 30500, "score should include large bonus");
    require(state.hiScore == 30500, "hi-score should update after large bonus");
    require(state.lives == 6, "lives should include all milestone awards");
}

static void testAddScoreIgnoresNonPositivePoints() {
    ScoreState state = {500, 700, 2};

    int extraLives = AddScore(state, 0, 10000);

    require(extraLives == 0, "zero points should not award lives");
    require(state.score == 500, "zero points should not change score");
    require(state.hiScore == 700, "zero points should not change hi-score");
    require(state.lives == 2, "zero points should not change lives");
}

static void testReleaseHumanoidUsesReleaseGroundForFallingState() {
    HumanoidReleaseState state = {100.f, 250.f, 600.f, 42.f, false, true};

    ReleaseHumanoid(state, 450.f, 525.f, true);

    require(state.wx == 450.f, "release should update world X");
    require(state.groundY == 525.f, "release should update ground height");
    require(state.y == 250.f, "falling release should preserve current Y");
    require(state.vy == 0.f, "release should reset fall velocity");
    require(state.falling, "falling release should enter falling state");
    require(!state.beingCarried, "release should clear carried state");
}

static void testReleaseHumanoidGroundsNonFallingState() {
    HumanoidReleaseState state = {100.f, 650.f, 600.f, 42.f, true, true};

    ReleaseHumanoid(state, 450.f, 525.f, false);

    require(state.wx == 450.f, "grounded release should update world X");
    require(state.groundY == 525.f, "grounded release should update ground height");
    require(state.y == 525.f, "grounded release should clamp below-ground Y");
    require(state.vy == 0.f, "grounded release should reset velocity");
    require(!state.falling, "grounded release should clear falling state");
    require(!state.beingCarried, "grounded release should clear carried state");
}

static void testBuildHiScorePathPrefersAppData() {
    std::string path = BuildHiScorePath("C:\\Users\\me\\AppData\\Roaming",
                                        "/home/me/.xdg-data", "/home/me", ".");

#ifdef _WIN32
    require(path == "C:\\Users\\me\\AppData\\Roaming\\Offender\\hiscore.dat" ||
            path == "C:\\Users\\me\\AppData\\Roaming/Offender/hiscore.dat",
            "hi-score path should prefer app data");
#else
    require(path == "/home/me/.xdg-data/offender/hiscore.dat",
            "non-Windows hi-score path should ignore app data");
#endif
}

static void testBuildHiScorePathUsesXdgBeforeHome() {
    std::string path = BuildHiScorePath("", "/home/me/.local/share", "/home/me", ".");

#ifdef _WIN32
    require(path == ".\\hiscore.dat", "Windows should ignore XDG and HOME values");
#else
    require(path == "/home/me/.local/share/offender/hiscore.dat" ||
            path == "/home/me/.local/share\\offender\\hiscore.dat",
            "hi-score path should use XDG data home before HOME");
#endif
}

static void testBuildHiScorePathFallsBackToWorkingDirectory() {
    std::string path = BuildHiScorePath("", "", "", ".");

    require(path == "./hiscore.dat" || path == ".\\hiscore.dat",
            "hi-score path should fall back to working directory");
}

int main() {
    testAddScoreUpdatesScoreAndHiScore();
    testAddScoreAwardsEachMilestoneCrossed();
    testAddScoreIgnoresNonPositivePoints();
    testReleaseHumanoidUsesReleaseGroundForFallingState();
    testReleaseHumanoidGroundsNonFallingState();
    testBuildHiScorePathPrefersAppData();
    testBuildHiScorePathUsesXdgBeforeHome();
    testBuildHiScorePathFallsBackToWorkingDirectory();
    return 0;
}
