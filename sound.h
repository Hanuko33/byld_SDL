#ifndef SOUND_H
#define SOUND_H

#ifndef ANDROID
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#else
#include <SDL.h>
#include <SDL_mixer.h>
#endif

enum sounds_enum {
    SOUND_dirt,
    SOUND_stone,
    SOUND_max
};

extern Mix_Chunk * sounds[SOUND_max];

void init_sounds();
void play_sound(enum sounds_enum sound);

#endif
