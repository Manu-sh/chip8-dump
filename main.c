#pragma ide diagnostic ignored "EndlessLoop"

#define _DEFAULT_SOURCE

//#define CHIP_DEBUG

#include <bit_utility.h>
#include <chip8.h>
#include <font.h>
#include <opcode.h>
#include <dbg.h>
#include <scale.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <signal.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_keyboard.h>

#include <chronos.h>

SDL_Palette * sdl_palette_new() {

    SDL_Palette *palette = SDL_CreatePalette(256);
    SDL_Color colors[256];

    memset(colors, 0x00, sizeof(colors)); // every other color than white is black
    memset(colors + 255, 0xff, sizeof(colors[0])); // this is white

    SDL_SetPaletteColors(palette, colors, 0, 256);

    return palette;
}

void sdl_palette_free(SDL_Palette *palette) {
    SDL_DestroyPalette(palette);
}


bool sdl_remap_key(SDL_Scancode keycode, uint8_t keypad[HKEY_LEN]) {

    switch (keycode) {
        case SDL_SCANCODE_0: keypad[HKEY_0] = PRESS; return true;
        case SDL_SCANCODE_1: keypad[HKEY_1] = PRESS; return true;
        case SDL_SCANCODE_2: keypad[HKEY_2] = PRESS; return true;
        case SDL_SCANCODE_3: keypad[HKEY_3] = PRESS; return true;
        case SDL_SCANCODE_4: keypad[HKEY_4] = PRESS; return true;
        case SDL_SCANCODE_5: keypad[HKEY_5] = PRESS; return true;
        case SDL_SCANCODE_6: keypad[HKEY_6] = PRESS; return true;
        case SDL_SCANCODE_7: keypad[HKEY_7] = PRESS; return true;
        case SDL_SCANCODE_8: keypad[HKEY_8] = PRESS; return true;
        case SDL_SCANCODE_9: keypad[HKEY_9] = PRESS; return true;
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
    SDL_Window *window = SDL_CreateWindow(
        "chip8 emulator",
        SCREEN_WIDTH  * SCALE,
        SCREEN_HEIGHT * SCALE,
        SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    //SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE); // sync with display HZ

    SDL_Surface *surface = SDL_CreateSurface(
        SCREEN_WIDTH  * SCALE,
        SCREEN_HEIGHT * SCALE,
        SDL_PIXELFORMAT_INDEX8
    );

    SDL_Palette *palette = sdl_palette_new();
    SDL_SetSurfacePalette(surface, palette);

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

        // Copia il framebuffer dentro la surface
        SDL_LockSurface(surface);
        memcpy(surface->pixels, scale_screen(chip), SCREEN_WIDTH * SCALE * SCREEN_HEIGHT * SCALE);
        SDL_UnlockSurface(surface);

        // Aggiorna la texture con il contenuto del framebuffer
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(texture); // rilascia texture temporanea

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
    SDL_DestroySurface(surface);
    sdl_palette_free(palette);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    chip_free(chip);
    return 0;
}
