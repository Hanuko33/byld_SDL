#ifndef TEXT_H
#define TEXT_H
#ifdef ANDROID
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
int load_font();
void write_text(int x, int y, const char * text, SDL_Color color, int scale, SDL_Window* window, SDL_Renderer* renderer);

#endif
