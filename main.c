#pragma ide diagnostic ignored "EndlessLoop"

#define _DEFAULT_SOURCE

//#define CHIP_DEBUG

#include <bit_utility.h>
#include <chip8.h>
#include <font.h>
#include <opcode.h>
#include <dbg.h>
#include <chronos.h>
#include <sdl.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <signal.h>


bool sdl_remap_key(SDL_Scancode keycode, uint8_t keypad[HKEY_LEN]) {

    switch (keycode) {
        case SDL_SCANCODE_KP_0: case SDL_SCANCODE_0: keypad[HKEY_0] = PRESS; return true;
        case SDL_SCANCODE_KP_1: case SDL_SCANCODE_1: keypad[HKEY_1] = PRESS; return true;
        case SDL_SCANCODE_KP_2: case SDL_SCANCODE_2: keypad[HKEY_2] = PRESS; return true;
        case SDL_SCANCODE_KP_3: case SDL_SCANCODE_3: keypad[HKEY_3] = PRESS; return true;
        case SDL_SCANCODE_KP_4: case SDL_SCANCODE_4: keypad[HKEY_4] = PRESS; return true;
        case SDL_SCANCODE_KP_5: case SDL_SCANCODE_5: keypad[HKEY_5] = PRESS; return true;
        case SDL_SCANCODE_KP_6: case SDL_SCANCODE_6: keypad[HKEY_6] = PRESS; return true;
        case SDL_SCANCODE_KP_7: case SDL_SCANCODE_7: keypad[HKEY_7] = PRESS; return true;
        case SDL_SCANCODE_KP_8: case SDL_SCANCODE_8: keypad[HKEY_8] = PRESS; return true;
        case SDL_SCANCODE_KP_9: case SDL_SCANCODE_9: keypad[HKEY_9] = PRESS; return true;
        case SDL_SCANCODE_A: keypad[HKEY_A] = PRESS; return true;
        case SDL_SCANCODE_B: keypad[HKEY_B] = PRESS; return true;
        case SDL_SCANCODE_C: keypad[HKEY_C] = PRESS; return true;
        case SDL_SCANCODE_D: keypad[HKEY_D] = PRESS; return true;
        case SDL_SCANCODE_E: keypad[HKEY_E] = PRESS; return true;
        case SDL_SCANCODE_F: keypad[HKEY_F] = PRESS; return true;
        default: break;
    }

    return false;
}

int main(int argc, char *argv[]) {

    assert(argc > 1);
    printf("argv[1] = \"%s\"\n", argv[1]);

    SDL_Init(SDL_INIT_VIDEO);
    sdl_t *sdl = sdl_new("chip8 emulator", SCREEN_WIDTH, SCREEN_HEIGHT, 10);

    assert(argc > 1);

    chip8_t *chip = chip_new();
    if (!chip_load_rom(chip, argv[1]))
        goto die;

    SDL_Event event;

    chronos_t timer60hz;
    chronos_start(&timer60hz);

    while (1) {
        if (SDL_PollEvent(&event)) {

            if (event.type == SDL_EVENT_QUIT)
                goto die;

            if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat)
                continue;

            if (sdl_remap_key(event.key.scancode, chip->keypad)) {
                #ifdef CHIP_DEBUG
                    printf("some key pressed\n");
                #endif
                //sleep(1);
            }
        }

        //dbg("PC: %#04x ", chip->PC);
        chip_exec(chip, chip_fetch(chip, chip->PC));

        sdl_sync_fb(sdl, chip->screen);
        sdl_render(sdl);

#ifdef CHIP_DEBUG
        printf("%s\n", byte_dump(chip->keypad, sizeof(chip->keypad)));
#endif

        if (chronos_elapsed(&timer60hz) > 16) {
            chip_tick(chip);
            chronos_restart(&timer60hz);
        }
    }

die:
    // Close window and OpenGL context
    sdl_free(sdl);
    SDL_Quit();
    chip_free(chip);
    return 0;
}
