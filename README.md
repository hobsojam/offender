# OFFENDER — Defender Clone

A faithful clone of the classic 1981 Williams arcade game, written in **C++ with raylib 5.5**.  
Native Windows executable, no external DLLs, keyboard only.

## Build

**Requirements:** GCC / w64devkit, CMake ≥ 3.20, Ninja, git.  
Raylib 5.5 is downloaded and compiled automatically via CMake FetchContent.

```
cmake -G Ninja -B build
cmake --build build
```

Output: `build\offender.exe` (~2.3 MB, self-contained)

For deterministic debugging runs, set `OFFENDER_SEED` before launching the game.

## Run

```
build\offender.exe
```

## Controls

| Key | Action |
|-----|--------|
| Left / Right arrows or A / D | Thrust horizontally (sets ship facing direction) |
| Up / Down arrows or W / S | Move vertically |
| Space or Left Ctrl | Fire laser |
| B or Z | Smart bomb — destroys all on-screen enemies (3 per wave) |
| H or Left Shift | Hyperspace — random teleport |
| P | Pause / Resume |
| Enter | Start / confirm screens |
| Esc | Quit |

## How to play

- **Landers** (red saucers) descend to abduct humanoids on the terrain. Shoot them before they reach the top.
- If a lander carries a humanoid to the top of the screen it becomes a **Mutant** — faster, more aggressive, and it hunts you.
- Shoot a carrying lander mid-flight → humanoid falls. Fly beneath it to catch it and earn a rescue bonus.
- **Baiters** appear after ~20 s per wave — thin yellow enemies that shoot rapidly and close fast.
- Lose all humanoids → every remaining lander mutates immediately (planet destroyed mode).
- **Radar** strip at the top shows the full 6400-pixel world: player (white), landers (red), mutants (purple), baiters (yellow), humanoids (green).
- Extra life every **10 000 points**. Start with 3 lives and 3 smart bombs per wave.

## Architecture

| File | Purpose |
|------|---------|
| `src/config.h` | All game constants |
| `src/entity.h` | Shared math helpers (`wrapX`, `wrapDX`, `wsX`) and types |
| `src/world.h/cpp` | Terrain generation, wraparound camera, star background |
| `src/player.h/cpp` | Ship physics, input, drawing |
| `src/enemies.h/cpp` | Lander / Mutant / Baiter AI and drawing |
| `src/humanoid.h/cpp` | Abduction, free-fall, rescue logic |
| `src/projectile.h` | Laser and enemy shot structs |
| `src/radar.h/cpp` | Minimap strip rendering |
| `src/audio.h/cpp` | Procedural 8-bit SFX (square waves, sweeps, arpeggios) |
| `src/hud.h/cpp` | Score, lives, wave, bomb display |
| `src/game.h/cpp` | Master state machine, collision detection, wave management |
| `src/main.cpp` | Window setup and game loop |
