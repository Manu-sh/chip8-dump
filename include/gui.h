#pragma once
#include <screen.h>
#include <raylib.h>

void init_gui() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "chip8 emulator");
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
}
