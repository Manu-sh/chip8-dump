#pragma once
#include <stdint.h>

/*

If the program attempts to draw a sprite at an x coordinate greater than 0x3F, the x value will be reduced modulo 64. Similarly, if the program attempts
to draw at a y coordinate greater than 0x1F, the y value will be reduced modulo 32. Sprites that are drawn partially off-screen will be clipped.
sixty-four by thirty-two pixel screen. By default, the screen is set to all black pixels.

The only method of drawing to the screen is using sprites.
CHIP-8 sprites are always 8 pixels wide and between one to fifteen pixels high.

Sprite data is stored in memory, just like CHIP-8 instructions themselves. One byte corresponds to one row of a given sprite.
How many rows (bytes) encompass a sprite is specified through the CHIP-8 instructions used to draw the sprites on the screen,
and will be covered later. For sprite data, a bit set to one corresponds to a white pixel.

Contrastingly, a bit set to zero corresponds to a transparent pixel.


How many rows (bytes) encompass a sprite is specified through the CHIP-8 instructions used to draw the sprites on the screen
or sprite data, a bit set to one corresponds to a white pixel. Contrastingly, a bit set to zero corresponds to a transparent pixel.
*/

#define SCREEN_WIDTH  64
#define SCREEN_HEIGHT 32

// r * width + c
#define SC(_X_,_Y_) ( _X_ * SCREEN_WIDTH + _Y_)


// TODO: servirebbe un piccolo wrapper per gestire lo scaling typedef { ... } screen_t;
#define SCALE 10

uint8_t * scale_screen(uint8_t *chip) {

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