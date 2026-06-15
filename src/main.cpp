#include "raylib.h"
#include "game.h"
#include "config.h"

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_W, SCREEN_H, "OFFENDER  -  Defender Clone");
    SetTargetFPS(60);
    InitAudioDevice();

    Game game;
    game.init();

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) break;
        if (IsKeyPressed(KEY_P))      game.togglePause();

        float dt = GetFrameTime();
        game.update(dt);

        BeginDrawing();
        ClearBackground(BLACK);
        game.draw();
        EndDrawing();
    }

    game.shutdown();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
