#include "random.h"

#include <random>

static std::mt19937& randomEngine() {
    static std::mt19937 engine{std::random_device{}()};
    return engine;
}

void SeedRandom(unsigned seed) {
    randomEngine().seed(seed);
}

float RandomFloat(float lo, float hi) {
    std::uniform_real_distribution<float> dist(lo, hi);
    return dist(randomEngine());
}
