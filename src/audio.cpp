#include "audio.h"
#include <cmath>
#include <cstdlib>

static Sound makeSound(float freq, float dur, float vol,
                       float freqEnd = -1.f, int waveType = 0)
{
    // waveType: 0=square, 1=sine, 2=noise, 3=sawtooth
    if (freqEnd < 0.f) freqEnd = freq;
    int sampleRate = 22050;
    int n = (int)(dur * sampleRate);
    if (n < 1) n = 1;

    short* data = (short*)malloc(n * sizeof(short));
    float phase = 0.f;

    for (int i = 0; i < n; i++) {
        float t   = (float)i / n;          // 0..1
        float env = 1.f - t;               // linear decay
        float f   = freq + (freqEnd - freq) * t;

        float sample = 0.f;
        switch (waveType) {
        case 0: // square
            sample = (sinf(phase) > 0.f) ? 1.f : -1.f;
            break;
        case 1: // sine
            sample = sinf(phase);
            break;
        case 2: // noise
            sample = ((float)rand() / (float)RAND_MAX) * 2.f - 1.f;
            break;
        case 3: // sawtooth
            sample = 2.f * (phase / (2.f * PI) - floorf(phase / (2.f * PI) + 0.5f));
            break;
        }
        data[i] = (short)(vol * env * 32767.f * sample);
        phase += 2.f * PI * f / sampleRate;
        if (phase > 2.f * PI) phase -= 2.f * PI;
    }

    Wave w;
    w.frameCount = (unsigned int)n;
    w.sampleRate = (unsigned int)sampleRate;
    w.sampleSize = 16;
    w.channels   = 1;
    w.data       = data;

    Sound s = LoadSoundFromWave(w);
    free(data);
    return s;
}

void AudioFX::init() {
    fire      = makeSound(880.f,  0.08f, 0.45f, 440.f,  0);  // short zap
    explode   = makeSound(120.f,  0.35f, 0.70f, 40.f,   2);  // noise burst
    abduct    = makeSound(440.f,  0.25f, 0.50f, 660.f,  1);  // rising sine
    rescue    = makeSound(660.f,  0.20f, 0.55f, 880.f,  1);  // happy blip
    bomb      = makeSound(80.f,   0.50f, 0.80f, 20.f,   2);  // big boom
    hyper     = makeSound(200.f,  0.30f, 0.60f, 800.f,  3);  // sawtooth sweep
    die       = makeSound(200.f,  0.55f, 0.75f, 50.f,   0);  // descending square
    extraLife = makeSound(523.f,  0.35f, 0.65f, 784.f,  1);  // C→G major 3rd
}

void AudioFX::shutdown() {
    UnloadSound(fire);
    UnloadSound(explode);
    UnloadSound(abduct);
    UnloadSound(rescue);
    UnloadSound(bomb);
    UnloadSound(hyper);
    UnloadSound(die);
    UnloadSound(extraLife);
}
