#pragma once
#include <stdint.h>
#include <assert.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>

// An 8 bit monochrome palette, this is required when the surface is 8 bit too (SDL_PIXELFORMAT_INDEX8)
static SDL_Palette * sdl_palette_monochrome_new() {

    SDL_Palette *palette = SDL_CreatePalette(256);
    SDL_Color colors[256];

    memset(colors, 0x00, sizeof(colors)); // every other color than white is black
    memset(colors + 255, 0xff, sizeof(colors[0])); // this is white

    SDL_SetPaletteColors(palette, colors, 0, 256);
    return palette;
}

static void sdl_palette_monochrome_free(SDL_Palette *palette) {
    SDL_DestroyPalette(palette);
}


typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Surface  *surface; // 8 bit surface
    SDL_Palette  *palette; // a custom palette required to use 8 bit depth

    SDL_Texture  *texture;

    uint16_t width, height;
    uint8_t scale;
} sdl_t;



sdl_t * sdl_new(const char *title, uint16_t width, uint16_t height, uint8_t scale) {

    sdl_t *self = calloc(1, sizeof(sdl_t));
    assert(self);

    self->width  = width;
    self->height = height;
    self->scale  = scale;

    self->window = SDL_CreateWindow(
        title,
        self->width * self->scale,
        self->height * self->scale,
        SDL_WINDOW_RESIZABLE
    );

    if (!self->window) {
        free(self);
        return NULL;
    }

    if (!(self->renderer = SDL_CreateRenderer(self->window, NULL))) {
        SDL_DestroyWindow(self->window);
        free(self);
        return NULL;
    }

    // (bool) Returns true on success or false on failure; call SDL_GetError() for more information.
    SDL_SetRenderScale(self->renderer, self->scale, self->scale);
    //SDL_SetRenderVSync(self->renderer, SDL_RENDERER_VSYNC_ADAPTIVE); // sync with display HZ

    self->surface = SDL_CreateSurface(
        self->width,
        self->height,
        SDL_PIXELFORMAT_INDEX8
    );

    if (!self->surface) {
        SDL_DestroyRenderer(self->renderer);
        SDL_DestroyWindow(self->window);
        free(self);
        return NULL;
    }


    if (!(self->palette = sdl_palette_monochrome_new())) {
        SDL_DestroySurface(self->surface);
        SDL_DestroyRenderer(self->renderer);
        SDL_DestroyWindow(self->window);
        free(self);
        return NULL;
    }

    // (int) Returns 0 on success or a negative error code on failure; call SDL_GetError() for more information.
    SDL_SetSurfacePalette(self->surface, self->palette);

    // a dummy texture
    self->texture = SDL_CreateTexture(
        self->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        self->width, self->height
    );

    if (!self->texture) {
        sdl_palette_monochrome_free(self->palette);
        SDL_DestroySurface(self->surface);
        SDL_DestroyRenderer(self->renderer);
        SDL_DestroyWindow(self->window);
        free(self);
        return NULL;
    }

    SDL_SetTextureScaleMode(self->texture, SDL_SCALEMODE_NEAREST);
    return self;
}

// chip_screen must be an aligned 32 pointer pointing to (width*height) bytes
void sdl_sync_fb(sdl_t *self, void *chip_screen) {

    SDL_LockSurface(self->surface); // Copia il framebuffer dentro la surface
    memcpy(__builtin_assume_aligned(self->surface->pixels, 32), chip_screen, self->width * self->height);
    SDL_UnlockSurface(self->surface);

    /*
    // Scala il framebuffer ospitato dalla surface
    SDL_Texture *texture = SDL_CreateTextureFromSurface(self->renderer, self->surface);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    */

    SDL_Surface *surf32 = SDL_ConvertSurface(self->surface, SDL_PIXELFORMAT_ARGB8888);
    SDL_UpdateTexture(self->texture, NULL, surf32->pixels, surf32->pitch);
    SDL_DestroySurface(surf32);
}


void sdl_render(sdl_t *self) {
    SDL_RenderClear(self->renderer);
    SDL_RenderTexture(self->renderer, self->texture, NULL, NULL);
    SDL_RenderPresent(self->renderer);
}


void sdl_free(sdl_t *self) {
    SDL_DestroySurface(self->surface);
    sdl_palette_monochrome_free(self->palette);
    SDL_DestroyTexture(self->texture);
    SDL_DestroyRenderer(self->renderer);
    SDL_DestroyWindow(self->window);
}
