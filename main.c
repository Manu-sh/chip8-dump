#pragma ide diagnostic ignored "EndlessLoop"

#define _DEFAULT_SOURCE

//#define CHIP_DEBUG

#include <chip8.h>
#include <chronos.h>
#include <sdl.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <sdl_buzzer.h>


bool sdl_remap_key(SDL_Scancode keycode, chip8_t *chip, keystate_t status) {

    switch (keycode) {
        case SDL_SCANCODE_KP_0: case SDL_SCANCODE_0: chip_press_key(chip, HKEY_0, status); return true;
        case SDL_SCANCODE_KP_1: case SDL_SCANCODE_1: chip_press_key(chip, HKEY_1, status); return true;
        case SDL_SCANCODE_KP_2: case SDL_SCANCODE_2: chip_press_key(chip, HKEY_2, status); return true;
        case SDL_SCANCODE_KP_3: case SDL_SCANCODE_3: chip_press_key(chip, HKEY_3, status); return true;
        case SDL_SCANCODE_KP_4: case SDL_SCANCODE_4: chip_press_key(chip, HKEY_4, status); return true;
        case SDL_SCANCODE_KP_5: case SDL_SCANCODE_5: chip_press_key(chip, HKEY_5, status); return true;
        case SDL_SCANCODE_KP_6: case SDL_SCANCODE_6: chip_press_key(chip, HKEY_6, status); return true;
        case SDL_SCANCODE_KP_7: case SDL_SCANCODE_7: chip_press_key(chip, HKEY_7, status); return true;
        case SDL_SCANCODE_KP_8: case SDL_SCANCODE_8: chip_press_key(chip, HKEY_8, status); return true;
        case SDL_SCANCODE_KP_9: case SDL_SCANCODE_9: chip_press_key(chip, HKEY_9, status); return true;
        case SDL_SCANCODE_A: chip_press_key(chip, HKEY_A, status); return true;
        case SDL_SCANCODE_B: chip_press_key(chip, HKEY_B, status); return true;
        case SDL_SCANCODE_C: chip_press_key(chip, HKEY_C, status); return true;
        case SDL_SCANCODE_D: chip_press_key(chip, HKEY_D, status); return true;
        case SDL_SCANCODE_E: chip_press_key(chip, HKEY_E, status); return true;
        case SDL_SCANCODE_F: chip_press_key(chip, HKEY_F, status); return true;
        default: break;
    }

    return false;
}

int main(int argc, char *argv[]) {

    assert(argc > 1);
    printf("argv[1] = \"%s\"\n", argv[1]);

    srand(time(0));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    sdl_t *sdl = sdl_new("chip8 emulator", SCREEN_WIDTH, SCREEN_HEIGHT, 10);
    sdl_buzzer_t *buzzer = sdl_buzzer_new();

    assert(argc > 1);

    chip8_t *chip = chip_new();
    if (!chip_load_rom(chip, argv[1]))
        goto die;

    SDL_Event event;

    chronos_t timer60hz;
    chronos_start(&timer60hz);

    while (1) {

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
				case SDL_EVENT_QUIT: goto die;
				case SDL_EVENT_KEY_DOWN:
                    if (event.key.repeat) continue;
					sdl_remap_key(event.key.scancode, chip, KEY_DOWN);
					continue;
				case SDL_EVENT_KEY_UP:
                    if (event.key.repeat) continue;
					sdl_remap_key(event.key.scancode, chip, KEY_UP);
					continue;
			}
        }

        //dbg("PC: %#04x ", chip->PC);
        chip_exec(chip, chip_fetch(chip, chip->PC));
        sdl_sync_fb(sdl, chip->screen);
        sdl_render(sdl);

#ifdef CHIP_DEBUG
        printf("%s\n", byte_dump(chip->keypad, sizeof(chip->keypad)));
#endif

        // TODO: fix display waiting OFF quirk
        if (chronos_elapsed(&timer60hz) > 16.6) {
            chip_tick(chip);
            if (chip->sound_timer) sdl_buzzer_beep(buzzer);
            chronos_restart(&timer60hz);
        }

        SDL_DelayNS(.8f * 1.0e6); // 0.8ms
        //SDL_DelayNS(16.6f * 1.0e6);
    }

die:
    // Close window and OpenGL context
    sdl_buzzer_free(buzzer);
    sdl_free(sdl);
    SDL_Quit();
    chip_free(chip);
    return EXIT_SUCCESS;
}
