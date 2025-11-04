#pragma once
#include <screen.h>
#include <raylib.h>

#define SCALE 10

void init_gui() {
    InitWindow(SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, "chip8 emulator");
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
}

uint8_t * scale_screen(chip8_t *chip) {

    if (SCALE == 1) return chip->screen;

    static uint8_t chip_screen[ (SCREEN_WIDTH * SCALE) * (SCREEN_HEIGHT * SCALE) ];

    for (int r = 0; r < SCREEN_HEIGHT; r++) {
        for (int c = 0; c < SCREEN_WIDTH; c++) {
            uint8_t pixel = chip->screen[ SC(r, c) ];

            for (int rs = 0; rs < SCALE; rs++) {
                for (int cs = 0; cs < SCALE; cs++) {
                    int dst_x = c * SCALE + cs;
                    int dst_y = r * SCALE + rs;
                    chip_screen[dst_y * (SCREEN_WIDTH * SCALE) + dst_x] = pixel;
                }
            }

        }
    }


    return chip_screen;

}