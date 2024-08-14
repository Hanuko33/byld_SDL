
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef ANDROID
#include <SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif

TTF_Font* font;

#define FONT_NAME "nerdfont.otf"


char status_line[256];
int status_code;

int load_font()
{
    font = TTF_OpenFont(FONT_NAME, 128);
    if (!(font)) {
        return 1;
        SDL_Log("problem with font");
    }
    else {
        SDL_Log("font loaded");
        return 0;
    }
}

void write_text(int x, int y, const char * text, SDL_Color color, int scale, SDL_Window* window, SDL_Renderer* renderer)
{
    SDL_Surface* surface;
	surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* text_sdl = SDL_CreateTextureFromSurface(renderer, surface);

	int x_size, y_size;
    x_size = strlen(text) * (scale) * 0.5;
    y_size = scale;

    SDL_Rect rect = {x, y, x_size, y_size};
    SDL_RenderCopy(renderer, text_sdl, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(text_sdl);
}


