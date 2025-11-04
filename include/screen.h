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

const uint8_t SCREEN_WIDTH  = 64; // i prefer const instead of a macro because i could need the address
const uint8_t SCREEN_HEIGHT = 32;
