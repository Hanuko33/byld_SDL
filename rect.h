#ifndef RECT_H
#define RECT_H
#ifdef ANDROID
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
void draw_rectangle(SDL_Renderer * renderer, int x,int y,int w,int h, SDL_Color c0, SDL_Color c1, SDL_Color c2, SDL_Color c3);
#endif
