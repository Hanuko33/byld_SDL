#include "sound.h"
#include <stdlib.h>

Mix_Chunk * sounds[SOUND_max];

void init_sounds()
{
    SDL_RWops * file = SDL_RWFromFile("sounds/dirt.wav", "rb");
    sounds[SOUND_dirt] = Mix_LoadWAV_RW(file, 0);
    file = SDL_RWFromFile("sounds/stone.wav", "rb");
    sounds[SOUND_stone] = Mix_LoadWAV_RW(file, 0);
}

void play_sound(enum sounds_enum sound)
{
    if (sound != SOUND_max)
        Mix_PlayChannel(0, sounds[sound], 0);
}


