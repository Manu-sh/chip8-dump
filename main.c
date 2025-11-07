#define _DEFAULT_SOURCE
#include <bit_utility.h>
#include <chip8.h>
#include <font.h>
#include <opcode.h>
#include <dbg.h>
#include <scale.h>

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_keyboard.h>


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

int main(int argc, char *argv[]) {

    assert(argc > 1);
    printf("argv[1] = \"%s\"\n", argv[1]);

    //SetTraceLogLevel(LOG_ERROR);
    //InitWindow(SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, "chip8 emulator");
    //SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow(
        "chip8 emulator",
        SCREEN_WIDTH  * SCALE,
        SCREEN_HEIGHT * SCALE,
        SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

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

    while (1) {

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_EVENT_QUIT)
                goto die;

            if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat)
                continue;

            switch (event.key.scancode) {
                case SDL_SCANCODE_0: chip->keypad[HKEY_0] = PRESS; break;
                case SDL_SCANCODE_1: chip->keypad[HKEY_1] = PRESS; break;
            }

            printf("some key pressed\n");
            sleep(1);
        }

/*
        for (int k; (k = GetKeyPressed()) != 0; ) {
            switch (k) {
                case KEY_ZERO:  chip->keypad[HKEY_0] = PRESS; break;
                case KEY_ONE:   chip->keypad[HKEY_1] = PRESS; break;
                case KEY_TWO:   chip->keypad[HKEY_2] = PRESS; break;
                case KEY_THREE: chip->keypad[HKEY_3] = PRESS; break;
                case KEY_FOUR:  chip->keypad[HKEY_4] = PRESS; break;
                case KEY_FIVE:  chip->keypad[HKEY_5] = PRESS; break;
                case KEY_SIX:   chip->keypad[HKEY_6] = PRESS; break;
                case KEY_SEVEN: chip->keypad[HKEY_7] = PRESS; break;
                case KEY_EIGHT: chip->keypad[HKEY_8] = PRESS; break;
                case KEY_NINE:  chip->keypad[HKEY_9] = PRESS; break;
                case KEY_A:     chip->keypad[HKEY_A] = PRESS; break;
                case KEY_B:     chip->keypad[HKEY_B] = PRESS; break;
                case KEY_C:     chip->keypad[HKEY_C] = PRESS; break;
                case KEY_D:     chip->keypad[HKEY_D] = PRESS; break;
                case KEY_E:     chip->keypad[HKEY_E] = PRESS; break;
                case KEY_F:     chip->keypad[HKEY_F] = PRESS; break;
            }
        }
*/
        // key_states[SDL_SCANCODE_W]

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

        printf("%s\n", byte_dump(chip->keypad, sizeof(chip->keypad)));

        //usleep(10 * 1000);
        chip_tick(chip);
        memset(chip->keypad, NOT_PRESS, sizeof(chip->keypad));
    }

    /* for (int k; (k = GetKeyPressed()) != 0; ) {
        switch (k) {
            case KEY_ZERO:  chip->keypad[HKEY_0] = PRESS; break;
            case KEY_ONE:   chip->keypad[HKEY_1] = PRESS; break;
            case KEY_TWO:   chip->keypad[HKEY_2] = PRESS; break;
            case KEY_THREE: chip->keypad[HKEY_3] = PRESS; break;
            case KEY_FOUR:  chip->keypad[HKEY_4] = PRESS; break;
            case KEY_FIVE:  chip->keypad[HKEY_5] = PRESS; break;
            case KEY_SIX:   chip->keypad[HKEY_6] = PRESS; break;
            case KEY_SEVEN: chip->keypad[HKEY_7] = PRESS; break;
            case KEY_EIGHT: chip->keypad[HKEY_8] = PRESS; break;
            case KEY_NINE:  chip->keypad[HKEY_9] = PRESS; break;
            case KEY_A:     chip->keypad[HKEY_A] = PRESS; break;
            case KEY_B:     chip->keypad[HKEY_B] = PRESS; break;
            case KEY_C:     chip->keypad[HKEY_C] = PRESS; break;
            case KEY_D:     chip->keypad[HKEY_D] = PRESS; break;
            case KEY_E:     chip->keypad[HKEY_E] = PRESS; break;
            case KEY_F:     chip->keypad[HKEY_F] = PRESS; break;
        }
    }*/

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
