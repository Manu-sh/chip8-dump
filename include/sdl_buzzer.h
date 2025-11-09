#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

#include <stdlib.h>

// the emulator buzzer to make beep(s)
typedef struct {
    SDL_AudioSpec spec;
    SDL_AudioStream *stream;
} sdl_buzzer_t;

sdl_buzzer_t * sdl_buzzer_new() {

    sdl_buzzer_t *self;
    if (!(self = calloc(1, sizeof(sdl_buzzer_t))))
        return NULL;

    self->spec.format   = SDL_AUDIO_F32;
    self->spec.channels = 2;
    self->spec.freq     = 8000;

    if (!(self->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &self->spec, NULL, NULL))) {
        free(self);
        return NULL;
    }

    SDL_ResumeAudioStreamDevice(self->stream); // SDL_OpenAudioDeviceStream starts the device paused. Resume it!
    return self;
}

// i copied this sample: https://github.com/libsdl-org/SDL/blob/main/examples/audio/02-simple-playback-callback/simple-playback-callback.c
void sdl_buzzer_beep(sdl_buzzer_t *self) {

    static int current_sine_sample = 0;

    /* see if we need to feed the audio stream more data yet.
       We're being lazy here, but if there's less than half a second queued, generate more.
       A sine wave is unchanging audio--easy to stream--but for video games, you'll want
       to generate significantly _less_ audio ahead of time! */
    const int minimum_audio = (self->spec.freq * sizeof(float)) / 2;  /* 8000 float samples per second. Half of that. */
    if (SDL_GetAudioStreamQueued(self->stream) < minimum_audio) {

        static float samples[512];  /* this will feed 512 samples each frame until we get to our maximum. */

        /* generate a 440Hz pure tone */
        for (unsigned i = 0; i < SDL_arraysize(samples); i++) {
            const int freq = 440;
            const float phase = current_sine_sample * freq / (float)self->spec.freq;
            samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
            current_sine_sample++;
        }

        /* wrapping around to avoid floating-point errors */
        current_sine_sample %= self->spec.freq;

        /* feed the new data to the stream. It will queue at the end, and trickle out as the hardware needs more data. */
        SDL_PutAudioStreamData(self->stream, samples, sizeof(samples));
    }


}

void sdl_buzzer_free(sdl_buzzer_t *self) {
    SDL_DestroyAudioStream(self->stream);
    free(self);
}
