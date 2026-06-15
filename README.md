# OFFENDER — Defender Clone

C++ / raylib native Windows game.

## Build

```
cmake -G Ninja -B build
cmake --build build
```
Requires: GCC (w64devkit), CMake, Ninja, git. Raylib 5.5 is fetched automatically.

## Run

```
build\offender.exe
```

## Controls

| Key | Action |
|-----|--------|
| Left / Right arrows or A / D | Thrust horizontally (sets facing direction) |
| Up / Down arrows or W / S | Move vertically |
| Space or Left Ctrl | Fire laser |
| B or Z | Smart bomb (3 per wave, clears screen) |
| H or Left Shift | Hyperspace (random teleport) |
| P | Pause / Resume |
| Enter | Start / confirm |
| Esc | Quit |

## Mechanics

- **Landers** descend to abduct humanoids. Shoot them before they reach the top.
- **Mutants** form when a lander carries a humanoid to the top — faster and aggressive.
- **Baiters** appear after ~20 seconds per wave — thin, fast, frequent shooters.
- Shoot a **carrying lander** → humanoid falls. Fly under it to catch it → rescue bonus.
- Lose all humanoids → all remaining landers mutate instantly.
- Extra life every **10 000 points**.
- **Radar** strip at top shows full world width.
