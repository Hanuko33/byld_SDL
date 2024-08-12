#include <SDL2/SDL.h>
#include <time.h>

void draw_rectangle(SDL_Renderer * renderer, int x,int y,int w,int h, SDL_Color c0, SDL_Color c1, SDL_Color c2, SDL_Color c3)
{
    SDL_Vertex v[4];
    int i[6] = {
        0,
        1,
        2,
        0,
        3,
        2
    };
    v[0].position.x = x;
    v[0].position.y = y;

    v[1].position.x = x+w;
    v[1].position.y = y;
    
    v[2].position.x = x+w;
    v[2].position.y = y+h;

    v[3].position.x = x;
    v[3].position.y = y+h;
    
    v[0].color = c0;
    v[1].color = c1;
    v[2].color = c2;
    v[3].color = c3;

    v[0].tex_coord.x=0;
    v[1].tex_coord.x=0;
    v[2].tex_coord.x=0;
    v[3].tex_coord.x=0;

    v[0].tex_coord.y=0;
    v[1].tex_coord.y=0;
    v[2].tex_coord.y=0;
    v[3].tex_coord.y=0;

    SDL_RenderGeometry(renderer, NULL, v, 6, i, 6);
}

