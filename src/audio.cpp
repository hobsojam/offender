#include "audio.h"
#include <cmath>
#include <cstdlib>

static const int SR = 44100;   // sample rate

// ── wave generators ────────────────────────────────────────────────────────
static inline float squareWave(float phase, float duty) {
    if (duty <= 0.f) return 0.f;
    if (duty >= 1.f) return 1.f;
    return fmodf(phase, 1.f) < duty ? 1.f : -1.f;
}
static inline float noiseWave() {
    return ((float)rand() / (float)RAND_MAX) * 2.f - 1.f;
}

// ── envelope ───────────────────────────────────────────────────────────────
// Simple AR envelope: attack fraction of total duration, linear decay after.
static inline float envelope(float t, float attackFrac) {
    if (t < attackFrac) return t / (attackFrac + 1e-6f);
    return 1.f - (t - attackFrac) / (1.f - attackFrac + 1e-6f);
}

// ── low-level buffer builder ───────────────────────────────────────────────
struct SynthParams {
    float freqA      = 440.f;  // start frequency
    float freqB      = -1.f;   // end frequency; -1 means same as freqA (flat tone)
    float duration   = 0.2f;
    float volume     = 0.6f;
    float duty       = 0.5f;   // square wave duty cycle
    float noiseMix   = 0.f;    // 0 = pure tone, 1 = pure noise
    float attackFrac = 0.02f;  // fraction of duration for attack ramp
    bool  expSweep   = true;   // exponential (musical) vs linear sweep
};

static Sound buildSound(const SynthParams& p) {
    int n = (int)(p.duration * SR);
    if (n < 1) n = 1;
    short* buf = (short*)malloc(n * sizeof(short));
    if (!buf) return Sound{};

    float freqB = (p.freqB < 0.f) ? p.freqA : p.freqB;
    float phase = 0.f;

    for (int i = 0; i < n; i++) {
        float t = (float)i / (n - 1 + 1e-6f);   // 0..1

        // Frequency sweep (exponential sounds musical, linear sounds synthetic)
        float freq;
        if (p.expSweep && p.freqA > 0.f && freqB > 0.f) {
            freq = p.freqA * powf(freqB / p.freqA, t);
        } else {
            freq = p.freqA + (freqB - p.freqA) * t;
        }

        float sq  = squareWave(phase, p.duty);
        float nz  = noiseWave();
        float sample = sq * (1.f - p.noiseMix) + nz * p.noiseMix;

        float env = envelope(t, p.attackFrac);
        buf[i] = (short)(p.volume * env * 32767.f * sample);

        phase += freq / SR;
    }

    Wave w = {};
    w.frameCount = (unsigned int)n;
    w.sampleRate = SR;
    w.sampleSize = 16;
    w.channels   = 1;
    w.data       = buf;
    Sound s = LoadSoundFromWave(w);
    free(buf);
    return s;
}

// Sequence of discrete notes (NES-style arpeggio)
static Sound buildArpeggio(const float* freqs, int noteCount,
                            float noteDur, float volume, float duty,
                            float noiseMix = 0.f) {
    int noteN = (int)(noteDur * SR);
    if (noteN < 1) noteN = 1;
    int total = noteN * noteCount;
    short* buf = (short*)malloc(total * sizeof(short));
    if (!buf) return Sound{};

    float phase = 0.f;
    for (int i = 0; i < total; i++) {
        int   note  = i / noteN;
        float noteT = (float)(i % noteN) / noteN;   // 0..1 within note
        float env   = 1.f - noteT;                  // decay per note

        float sq = squareWave(phase, duty);
        float nz = noiseWave();
        float sample = sq * (1.f - noiseMix) + nz * noiseMix;

        buf[i] = (short)(volume * env * 32767.f * sample);
        phase += freqs[note] / SR;
    }

    Wave w = {};
    w.frameCount = (unsigned int)total;
    w.sampleRate = SR;
    w.sampleSize = 16;
    w.channels   = 1;
    w.data       = buf;
    Sound s = LoadSoundFromWave(w);
    free(buf);
    return s;
}

// ── sound definitions ──────────────────────────────────────────────────────
void AudioFX::init() {

    // FIRE — sharp descending "pew", 25% duty for thin retro feel
    fire = buildSound({
        .freqA = 880.f, .freqB = 180.f,
        .duration = 0.09f, .volume = 0.5f, .duty = 0.25f,
        .noiseMix = 0.f, .attackFrac = 0.0f, .expSweep = true
    });

    // EXPLODE — noise burst with low bass kick underneath
    {
        int n = (int)(0.38f * SR);
        short* buf = (short*)malloc(n * sizeof(short));
        if (buf) {
            float phase = 0.f;
            for (int i = 0; i < n; i++) {
                float t   = (float)i / n;
                float env = 1.f - t;
                // Descending bass tone (adds thud) mixed with noise
                float freq = 120.f * powf(30.f / 120.f, t);
                float tone = squareWave(phase, 0.5f);
                float nz   = noiseWave();
                float s    = tone * 0.3f + nz * 0.7f;
                buf[i] = (short)(0.75f * env * 32767.f * s);
                phase += freq / SR;
            }
            Wave w = {}; w.frameCount=(unsigned)n; w.sampleRate=SR;
            w.sampleSize=16; w.channels=1; w.data=buf;
            explode = LoadSoundFromWave(w);
            free(buf);
        }
    }

    // ABDUCT — descending wail, lander has grabbed a humanoid
    abduct = buildSound({
        .freqA = 600.f, .freqB = 120.f,
        .duration = 0.28f, .volume = 0.55f, .duty = 0.5f,
        .noiseMix = 0.1f, .attackFrac = 0.04f, .expSweep = true
    });

    // RESCUE — ascending C-E-G arpeggio, cheerful
    {
        float notes[] = { 523.f, 659.f, 784.f };
        rescue = buildArpeggio(notes, 3, 0.065f, 0.55f, 0.125f);
    }

    // BOMB — heavy sub-bass noise blast
    {
        int n = (int)(0.55f * SR);
        short* buf = (short*)malloc(n * sizeof(short));
        if (buf) {
            float phase = 0.f;
            for (int i = 0; i < n; i++) {
                float t   = (float)i / n;
                float env = powf(1.f - t, 0.6f);        // slower decay for weight
                float freq = 80.f * powf(15.f / 80.f, t);
                float tone = squareWave(phase, 0.5f);
                float nz   = noiseWave();
                float s    = tone * 0.4f + nz * 0.6f;
                buf[i] = (short)(0.85f * env * 32767.f * s);
                phase += freq / SR;
            }
            Wave w = {}; w.frameCount=(unsigned)n; w.sampleRate=SR;
            w.sampleSize=16; w.channels=1; w.data=buf;
            bomb = LoadSoundFromWave(w);
            free(buf);
        }
    }

    // HYPERSPACE — rising sweep + glitchy noise, 50% duty sawtooth-ish
    {
        int n = (int)(0.32f * SR);
        short* buf = (short*)malloc(n * sizeof(short));
        if (buf) {
            float phase = 0.f;
            for (int i = 0; i < n; i++) {
                float t   = (float)i / n;
                float env = (t < 0.1f) ? t / 0.1f : 1.f - (t - 0.1f) / 0.9f;
                float freq = 80.f * powf(2200.f / 80.f, t);
                float tone = squareWave(phase, 0.5f);
                float nz   = noiseWave();
                // Noise fades in as pitch rises (like tearing through space)
                float mix  = t * 0.6f;
                float s    = tone * (1.f - mix) + nz * mix;
                buf[i] = (short)(0.65f * env * 32767.f * s);
                phase += freq / SR;
            }
            Wave w = {}; w.frameCount=(unsigned)n; w.sampleRate=SR;
            w.sampleSize=16; w.channels=1; w.data=buf;
            hyper = LoadSoundFromWave(w);
            free(buf);
        }
    }

    // DIE — descending arpeggio G4→E4→C4→A3, slow and mournful
    {
        float notes[] = { 392.f, 330.f, 262.f, 220.f };
        die = buildArpeggio(notes, 4, 0.12f, 0.7f, 0.5f, 0.15f);
    }

    // EXTRA LIFE — ascending C5→E5→G5→C6, bright 12.5% duty
    {
        float notes[] = { 523.f, 659.f, 784.f, 1047.f };
        extraLife = buildArpeggio(notes, 4, 0.075f, 0.6f, 0.125f);
    }
}

void AudioFX::shutdown() {
    UnloadSound(fire);      fire      = {};
    UnloadSound(explode);   explode   = {};
    UnloadSound(abduct);    abduct    = {};
    UnloadSound(rescue);    rescue    = {};
    UnloadSound(bomb);      bomb      = {};
    UnloadSound(hyper);     hyper     = {};
    UnloadSound(die);       die       = {};
    UnloadSound(extraLife); extraLife = {};
}
