#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

#include <stdlib.h>

// the emulator buzzer to make beep(s)
typedef struct {
    SDL_AudioSpec spec;
    SDL_AudioStream *stream;

    struct {
        int current_sine_sample; // 0
        float samples[512];
        int minimum_audio; // freq float samples per second. Half of that.
    };
} sdl_buzzer_t;

sdl_buzzer_t * sdl_buzzer_new() {

    sdl_buzzer_t *self;
    if (!(self = calloc(1, sizeof(sdl_buzzer_t))))
        return NULL;

    self->spec.format   = SDL_AUDIO_F32;
    self->spec.channels = 2;
    self->spec.freq     = 8000;

    self->minimum_audio = (self->spec.freq * sizeof(float)) / 2;

    if (!(self->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &self->spec, NULL, NULL))) {
        free(self);
        return NULL;
    }

    SDL_ResumeAudioStreamDevice(self->stream); // SDL_OpenAudioDeviceStream starts the device paused. Resume it!
    return self;
}

// i copied this sample: https://github.com/libsdl-org/SDL/blob/main/examples/audio/02-simple-playback-callback/simple-playback-callback.c
void sdl_buzzer_beep(sdl_buzzer_t *self) {

    /* see if we need to feed the audio stream more data yet.
       We're being lazy here, but if there's less than half a second queued, generate more.
       A sine wave is unchanging audio--easy to stream--but for video games, you'll want
       to generate significantly _less_ audio ahead of time! */
    if (SDL_GetAudioStreamQueued(self->stream) < self->minimum_audio) {

        /* this will feed 512 samples each frame until we get to our maximum. */

        /* generate a 440Hz pure tone */
        for (unsigned i = 0; i < SDL_arraysize(self->samples); i++) {
            static const int freq = 440;
            const float phase = self->current_sine_sample * freq / (float)self->spec.freq;
            self->samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
            self->current_sine_sample++;
        }

        /* wrapping around to avoid floating-point errors */
        self->current_sine_sample %= self->spec.freq;

        /* feed the new data to the stream. It will queue at the end, and trickle out as the hardware needs more data. */
        SDL_PutAudioStreamData(self->stream, self->samples, sizeof(self->samples));
    }


}

void sdl_buzzer_free(sdl_buzzer_t *self) {
    SDL_DestroyAudioStream(self->stream);
    free(self);
}
