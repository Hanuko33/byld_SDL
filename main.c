#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include "text.h"
#include "list/list.h"

int debug = 0;

SDL_Window *window;
SDL_Renderer *renderer;
struct Player
{
    char going_right;
    char no_clip;
    int x;
    int y;
    int speed;
};

struct Player player;
SDL_Texture* playerr_texture;
SDL_Texture* playerl_texture;

enum tiles
{
    TILE_blue_wall,
    TILE_green_wall,
    TILE_orange_wall,
    TILE_pink_wall,
    TILE_red_wall,
    TILE_violet_wall,
    TILE_yellow_wall,
    TILE_max
};

enum tiles current_tile = TILE_blue_wall;

char tile_solid[] = {
    1, // TILE_blue_wall
    1, // TILE_green_wall
    1, // TILE_orange_wall
    1, // TILE_pink_wall
    1, // TILE_red_wall
    1, // TILE_violet_wall
    1, // TILE_yellow_wall
};

SDL_Texture* tile_textures[TILE_max];

struct Tile
{
    int x;
    int y;
    int id;
};

struct Tile* Tile_create(int _x, int _y, int _id)
{
    struct Tile * t = malloc(sizeof(struct Tile));
    t->x = _x;
    t->y = _y;
    t->id = _id;
    return t;
}

struct List* world;

SDL_Texture* load_texture(const char * texture_name)
{
    SDL_Texture* texture = NULL;
    SDL_Surface* loadedSurface = IMG_Load(texture_name); 
    if (loadedSurface == NULL)
    {
        printf("Unable to load texture: %s error: %s\n", texture_name, SDL_GetError()); 
            exit(0);
    }
    else 
    {    
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);

        if (texture == NULL)
        {
            printf("Unable to create texture: %s error: %s\n", texture_name, SDL_GetError());
            exit(0);
        }
        SDL_FreeSurface(loadedSurface);
    }

    return texture;
}


int init_sdl2()
{
    window = SDL_CreateWindow("Platforer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 1024, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        return 1;
    }
    
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) 
    {
        printf("\nUnable to initialize sdl_image:  %s\n", IMG_GetError());
        return 1;
    }

    if (TTF_Init() == -1)
    {
        printf("\nUnable to initialize sdl_ttf:  %s\n", TTF_GetError());
        return 1;
    }

    SDL_Surface* icon = IMG_Load("textures/icon.png");
    if (icon == NULL) {
        printf("\nUnable to load image %s! SDL_image Error: %s\n", "textures/icon.png", IMG_GetError());
        return 1;
    }
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);


    return 0;
}

void save()
{
    FILE * f = fopen("world", "w");

    SDL_Rect img_rect = {};
    struct List * current = world;
    struct Tile * current_tile;
    char buf[16] = {};
    for (;;) {
        current_tile = ((struct Tile *)(current->var));

        fprintf(f, "%d %d %d\n", current_tile->x, current_tile->y, current_tile->id);

        if (current->next)
            current=current->next;
        else 
            break;
    }

    fclose(f);
}

void load()
{
    player.x=2110;
    player.y=2110;
    FILE * f = fopen("world", "r");
    int x,y,id;

    if (world)
    {
        free(world);
    }
    world = List_create();
    
    int status = fscanf(f, "%d %d %d", &x, &y, &id);
    if (status != 3)
    {
        fclose(f);
        return;
    }
    world->var = Tile_create(x, y, id);

    while (fscanf(f, "%d %d %d", &x, &y, &id) == 3) {
        List_append(world, Tile_create(x, y, id));
    }

    fclose(f);
}


void draw()
{
    // Player draw
    SDL_Rect player_rect = {480, 480, 64, 64};
    if (player.going_right)
        SDL_RenderCopy(renderer, playerr_texture, NULL, &player_rect);
    else
        SDL_RenderCopy(renderer, playerl_texture, NULL, &player_rect);

    // Tile draw
    if (world->var)
    {
        SDL_Rect img_rect = {};
        struct List * current = world;
        struct Tile * current_tile;
        for (;;) {
            current_tile = ((struct Tile *)(current->var));
            img_rect.x=current_tile->x*64-player.x+480;
            img_rect.y=current_tile->y*64-player.y+480;
            img_rect.w=64;
            img_rect.h=64;
            SDL_RenderCopy(renderer, tile_textures[current_tile->id], NULL, &img_rect);

            if (current->next)
                current=current->next;
            else 
                break;
        }
    }
    // Text draw
    char text[256];
    int text_y=16;
    
    sprintf(text, "X: %d (%d)", player.x, player.x/64);
    write_text(10, text_y, text, (SDL_Color){255,255,255,255}, 20, window, renderer);
    text_y+=16;

    sprintf(text, "Y: %d (%d)", player.y, player.y/64);
    write_text(10, text_y, text, (SDL_Color){255,255,255,255}, 20, window, renderer);
    text_y+=16;

    sprintf(text, "No clip: %s", player.no_clip ? "YES" : "NO" );
    write_text(10, text_y, text, (SDL_Color){255,255,255,255}, 20, window, renderer);
    text_y+=16;

    sprintf(text, "Speed: %s", player.speed==20 ? "2" : player.speed==10 ? "1" : "0.5" );
    write_text(10, text_y, text, (SDL_Color){255,255,255,255}, 20, window, renderer);
    text_y+=16;



    // Item menu draw
    for (int i = 0; i < TILE_max; i++) {
        SDL_Rect img_rect = {i*32+200, 10, 32, 32};
        SDL_RenderCopy(renderer, tile_textures[i], NULL, &img_rect);
    }
    SDL_Rect img_rect = {current_tile*32+200, 10, 32, 32};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &img_rect);

    // current tile draw
    sprintf(text, "Current tile: ");
    write_text(10, 100, text, (SDL_Color){255,255,255,255}, 20, window, renderer);
    img_rect = (SDL_Rect){150, 100, 32, 32};
    SDL_RenderCopy(renderer, tile_textures[current_tile], NULL, &img_rect);
}

enum Collision_id
{
    COLL_no,
    COLL_up,
    COLL_down,
    COLL_right,
    COLL_left,
};

int in_between(int a, int min)
{
    if (a >= min && a <= min+64)
        return 1;
    return 0;
}
int arst=0;
enum Collision_id get_collision(SDL_Rect o1, SDL_Rect o2, enum Collision_id filter)
{
    if ((in_between(o1.x+32, o2.x) || in_between(o1.x+32, o2.x)) && (filter == COLL_down || filter == COLL_no))
    {
        if (in_between(o1.y+60, o2.y))
            return COLL_down;
    }

    if ((in_between(o1.x+32, o2.x) || in_between(o1.x+32, o2.x)) && (filter == COLL_up || filter == COLL_no))
    {
        if (in_between(o1.y-4, o2.y))
            return COLL_up;
    }

    if ((in_between(o1.y+3, o2.y) || in_between(o1.y+50, o2.y)) && (filter == COLL_right || filter == COLL_no))
    {
        if (o1.x+40 >= o2.x && o1.x <= o2.x)
        {
            return COLL_right;
        }
    }

    if ((in_between(o1.y+3, o2.y) || in_between(o1.y+50, o2.y)) && (filter == COLL_left || filter == COLL_no))
    {
        if (o1.x >= o2.x && o1.x <= o2.x+40)
        {
            return COLL_left;
        }
    }

    return COLL_no;
}

int player_check_tile_collision(enum Collision_id id)
{
    if (world->var)
    {
        struct List * current = world;
        struct Tile * current_tile;
        for (;;) {
            current_tile = ((struct Tile *)(current->var));
            if (tile_solid[current_tile->id] && get_collision
                (
                    (SDL_Rect){player.x, player.y, 64, 64},
                    (SDL_Rect){current_tile->x*64, current_tile->y*64, 64, 64},
                    id
                ))
            {
                return 1;
            }
            if (current->next)
                current=current->next;
            else 
                return 0;
        }
    }
    else {
        printf("No world!\n");
    }
    return 0;
}


struct List * player_tile_collision(enum Collision_id id, enum tiles Tile_ID)
{
    if (world->var)
    {
        struct List * current = world;
        struct Tile * current_tile;
        for (;;) {
            current_tile = ((struct Tile *)(current->var));
            if (current_tile->id == Tile_ID && get_collision
                (
                    (SDL_Rect){player.x, player.y, 64, 64},
                    (SDL_Rect){current_tile->x*64, current_tile->y*64, 64, 64},
                    id
                ))
            {
                return current;
            }
            if (current->next)
                current=current->next;
            else 
                return NULL;
        }
    }
    else {
        printf("No world!\n");
    }
    return NULL;
}

void update(const Uint8 * keys)
{
    if (keys[SDL_SCANCODE_LSHIFT])
        player.speed = 5;
    else if (keys[SDL_SCANCODE_LCTRL])
        player.speed = 20;
    else
        player.speed = 10;


    if (keys[SDL_SCANCODE_D])
    {
        player.going_right = 1;
        player.x += player.speed;
    }

    if (keys[SDL_SCANCODE_A])
    {
        player.going_right = 0;
        player.x -= player.speed;
    }
    if (keys[SDL_SCANCODE_W])
        player.y -= player.speed;
    if (keys[SDL_SCANCODE_S])
        player.y += player.speed;


    if (!player.no_clip)
    {
        if (player_check_tile_collision(COLL_down))
            player.y -= player.speed;
        if (player_check_tile_collision(COLL_right))
            player.x -= player.speed;
        if (player_check_tile_collision(COLL_up))
            player.y += player.speed;
        if (player_check_tile_collision(COLL_left))
            player.x += player.speed;
    }
}

int main()
{
    player.no_clip=0;
    load();

    if (init_sdl2() != 0)
    {
        SDL_Quit();
        return 1;
    }
    load_font();
    playerr_texture = load_texture("textures/playerr.png");
    playerl_texture = load_texture("textures/playerl.png");
    tile_textures[TILE_blue_wall] = load_texture("textures/blue_wall.png");
    tile_textures[TILE_red_wall] = load_texture("textures/red_wall.png");
    tile_textures[TILE_pink_wall] = load_texture("textures/pink_wall.png");
    tile_textures[TILE_green_wall] = load_texture("textures/green_wall.png");
    tile_textures[TILE_orange_wall] = load_texture("textures/orange_wall.png");
    tile_textures[TILE_violet_wall] = load_texture("textures/violet_wall.png");
    tile_textures[TILE_yellow_wall] = load_texture("textures/yellow_wall.png");

    for (;;)
    {
        // Keyboard handling by events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                SDL_Quit();
                return 0;
            }
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        SDL_Quit();
                        return 0;
                    case SDLK_x:
                        player.no_clip ^= 1;
                        break;
                    case SDLK_F1:
                        debug ^= 1;
                        break;
                    case SDLK_F2:
                        save();
                        break;
                    case SDLK_F3:
                        load();
                        break;
                    case SDLK_COMMA:
                        current_tile--;
                        if (current_tile==-1)
                            current_tile=TILE_max-1;
                        break;
                    case SDLK_PERIOD:
                        current_tile++;
                        if (current_tile==TILE_max)
                            current_tile = 0;
                        break;
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (event.button.button == 1)
                {
                    List_append(world,
                                Tile_create(
                                        (x+player.x-480)/64,
                                        (y+player.y-480)/64,
                                    current_tile));
                }
                if (event.button.button == 3)
                {
                    struct List * current = world;
                    struct Tile * current_tile;
                    for (;;) 
                    {
                        current_tile = ((struct Tile *)(current->var));

                        if ((current_tile->x == (x+player.x-480)/64) && (current_tile->y == (y+player.y-480)/64))
                        {
                            if (current == world)
                            {
                                world = current->next;
                                current = current->next;
                                List_delete(current->previous);
                            }
                            else if (current->next)
                            {
                                current = current->next;
                                List_delete(current->previous);
                            }
                            else if (current->previous)
                            {
                                current = current->previous;
                                List_delete(current->next);
                            }
                        }

                        if (current->next)
                            current=current->next;
                        else
                            break;
                    }
                }
            }
        }

        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Keyboard handling by states
        const Uint8 * keys = SDL_GetKeyboardState(NULL);
        update(keys);
        draw();

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }
}

        
