#pragma once
#include <raylib.h>
#include <screen.h>
#include <scale.h>

void init_gui() {
    InitWindow(SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, "chip8 emulator");
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
}
