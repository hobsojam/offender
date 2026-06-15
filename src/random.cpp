#include "random.h"

#include <random>

static std::mt19937& randomEngine() {
    // Gameplay RNG only; not suitable for security or cryptographic use.
    static std::mt19937 engine;
    return engine;
}

void SeedRandom(unsigned seed) {
    randomEngine().seed(seed);
}

float RandomFloat(float lo, float hi) {
    std::uniform_real_distribution<float> dist(lo, hi);
    return dist(randomEngine());
}
