#ifdef ANDROID
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_scancode.h>
#include <SDL_timer.h>
#include <SDL_video.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#else
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "text.h"
#include "list/list.h"
#include "world.h"
#include "rect.h"
#include "sound.h"

int debug = 0;
int fps = 60;
int HOLD=0;
int RHOLD=0;
#ifdef ANDROID
int android_mode = 1;
#endif
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

enum tiles current_tile = 0;
SDL_Texture* tile_sheet;

SDL_Texture* im_select_texture;

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
        SDL_Log("Unable to load texture: %s error: %s\n", texture_name, SDL_GetError()); 
            exit(0);
    }
    else 
    {    
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);

        if (texture == NULL)
        {
            SDL_Log("Unable to create texture: %s error: %s\n", texture_name, SDL_GetError());
            exit(0);
        }
        SDL_FreeSurface(loadedSurface);
    }

    return texture;
}


int init_sdl2()
{
    SDL_Log("sdl_init start");
#ifdef ANDROID
    window = SDL_CreateWindow("Platforer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 720, 1280, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
#else
    window = SDL_CreateWindow("Platforer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
#endif
    SDL_Log("sdl_init create window");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Log("sdl_init create renderer");

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        return 1;
    }
    SDL_Log("sdl_init");

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) 
    {
        SDL_Log("\nUnable to initialize sdl_image:  %s\n", IMG_GetError());
        return 1;
    }

    if ( Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        SDL_Log("Mix_OpenAudio() failed: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1)
    {
        SDL_Log("\nUnable to initialize sdl_ttf:  %s\n", TTF_GetError());
        return 1;
    }

    SDL_Surface* icon = IMG_Load("textures/icon.png");
    if (icon == NULL) {
        SDL_Log("\nUnable to load image %s! SDL_image Error: %s\n", "textures/icon.png", IMG_GetError());
        return 1;
    }
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);


    return 0;
}

void save()
{
    SDL_RWops* file = SDL_RWFromFile("world", "wb");

    if (!file)
    {
        SDL_Log("+++++++++++ FILE SAVE FAILED: FILE CAN'T BE CREATED/OPENED TO WRITE");
        return;
    }

    struct List * current = world;
    struct Tile * current_tile;
    int data[3];
    for (;;) {
        current_tile = ((struct Tile *)(current->var));

        data[0]=current_tile->x;
        data[1]=current_tile->y;
        data[2]=current_tile->id;

        SDL_RWwrite(file, data, sizeof(int), 3);

        if (current->next)
            current=current->next;
        else 
            break;
    }

    SDL_RWclose(file);
}

void load()
{
    player.x=2110;
    player.y=2110;

    SDL_Log("-------------------------------------------- LOAD START");

    SDL_RWops * file = SDL_RWFromFile("world", "rb");

    if (!file) {
        SDL_Log("--------------------------------------------- NO WORLD FILE: USING DEFAULT");
        world = List_create();
        world->var = Tile_create(32,32,0);
        List_append(world, Tile_create(33, 33, 0));
        return;
    }
    SDL_RWseek(file, 0, RW_SEEK_SET);
    SDL_Log("-------------------------------------------- HAS FILE");
    int info[3]; // 0 = x, 1 = y 2 = id

    if (world)
    {
        free(world);
    }
    world = List_create();
    SDL_Log("-------------------------------------------- RECREATES WORLD");
    
    int status = SDL_RWread(file, info, sizeof(int), 3);
    SDL_Log("--------------------------------------------  SDL_RWread");
    if (status != 3)
    {
        SDL_Log("------------------------- EMPTY WORLD FILE: USING DEFAULT");
        SDL_RWclose(file);
        world->var = Tile_create(32,32,0);
        List_append(world, Tile_create(33, 33, 0));
        return;
    }
    SDL_Log("-------------------------------------------- LOADING FIRST TILE");
    world->var = Tile_create(info[0], info[1], info[2]);
    SDL_Log("-------- %d, %d, %d!!!!", info[0], info[1], info[2]);
    SDL_Log("-------------------------------------------- LOADED");

    SDL_Log("-------------------------------------------- STARTING LOAD LOOP");
    while (SDL_RWread(file, info, sizeof(int), 3) == 3) {
        List_append(world, Tile_create(info[0], info[1], info[2]));
        SDL_Log("-------- %d, %d, %d!!!!", info[0], info[1], info[2]);
    }
    SDL_Log("-------------------------------------------- ENDING LOAD LOOP");

    SDL_RWclose(file);
    SDL_Log("-------------------------------------------- ENDING LOAD FUNCTION");
}


void draw()
{

    int win_h,win_w;
    SDL_GetWindowSize(window, &win_w, &win_h);
    // GAME

    // Tile draw
    if (world->var)
    {
        SDL_Rect img_rect = {};
        SDL_Rect src_rect = {};
        struct List * current = world;
        struct Tile * current_tile;
        for (;;) {
            current_tile = ((struct Tile *)(current->var));
            img_rect.x=current_tile->x*64-player.x+(win_w/2-32);
            img_rect.y=current_tile->y*64-player.y+(win_h/2-32);
            img_rect.w=64;
            img_rect.h=64;
            if (img_rect.x < win_w && img_rect.y < win_h && img_rect.x > -100 && img_rect.y > -100)
            {
                src_rect = (SDL_Rect){32*current_tile->id, 0, 32, 32};
                SDL_RenderCopy(renderer, tile_sheet, &src_rect, &img_rect);
            }
            if (current->next)
                current=current->next;
            else 
                break;
        }
    }

    // Player draw
    SDL_Rect player_rect = {win_w/2-32, win_h/2-32, 64, 64};
    if (player.going_right)
        SDL_RenderCopy(renderer, playerr_texture, NULL, &player_rect);
    else
        SDL_RenderCopy(renderer, playerl_texture, NULL, &player_rect);

    // GUI
    // Background draw
    draw_rectangle(renderer, 0, 0, 600, 150,
                   (SDL_Color){0, 0, 155},
                   (SDL_Color){0, 155, 0},
                   (SDL_Color){55, 155, 155},
                   (SDL_Color){0, 0, 0});

    // Text draw
    char text[256];
    int text_y=16;

    sprintf(text, "FPS: %d", fps);
    write_text(10, text_y, text, (SDL_Color){255,255,255,255}, 20, window, renderer);
    text_y+=16;
    
    sprintf(text, "X: %d (%d)", player.x, player.x/64);
    write_text(10, text_y, text, (SDL_Color){255,255,255,255}, 20, window, renderer);
    text_y+=16;

    sprintf(text, "Y: %d (%d)", player.y, player.y/64);
    write_text(10, text_y, text, (SDL_Color){255,255,255,255}, 20, window, renderer);
    text_y+=16;

    sprintf(text, "No clip: %d", player.no_clip);
    write_text(10, text_y, text, (SDL_Color){255,255,255,255}, 20, window, renderer);
    text_y+=16;

    // Item menu draw
    for (int i = 0; i < TILE_max; i++) {
        if (i > 10)
        {
            SDL_Rect img_rect = {i*32-152, 42, 32, 32};
            SDL_Rect src_rect = (SDL_Rect){32*tiles_order[i], 0, 32, 32};
            SDL_RenderCopy(renderer, tile_sheet, &src_rect, &img_rect);
        }
        else
        {
            SDL_Rect img_rect = {i*32+200, 10, 32, 32};
            SDL_Rect src_rect = (SDL_Rect){32*tiles_order[i], 0, 32, 32};
            SDL_RenderCopy(renderer, tile_sheet, &src_rect, &img_rect);
        }
    }
    if (current_tile > 10)
    {
        SDL_Rect img_rect = {current_tile*32-152, 42, 32, 32};
        SDL_RenderCopy(renderer, im_select_texture, NULL, &img_rect);
    }
    else
    {
        SDL_Rect img_rect = {current_tile*32+200, 10, 32, 32};
        SDL_RenderCopy(renderer, im_select_texture, NULL, &img_rect);
    }
    // current tile draw
    sprintf(text, "Current tile: ");
    write_text(10, 100, text, (SDL_Color){255,255,255,255}, 20, window, renderer);
    SDL_Rect img_rect = {150, 100, 32, 32};
    SDL_Rect src_rect = {32*tiles_order[current_tile], 0, 32, 32};
    SDL_RenderCopy(renderer, tile_sheet, &src_rect, &img_rect);


#ifdef ANDROID
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // MOBILE OVERLAY
    SDL_Rect button_rect = {10, win_h-190, 90, 90};
    SDL_RenderCopy(renderer, im_select_texture, NULL, &button_rect);
    button_rect = (SDL_Rect){100, win_h-100, 90, 90};
    SDL_RenderCopy(renderer, im_select_texture, NULL, &button_rect);
    button_rect = (SDL_Rect){100, win_h-280, 90, 90};
    SDL_RenderCopy(renderer, im_select_texture, NULL, &button_rect);
    button_rect = (SDL_Rect){190, win_h-190, 90, 90};
    SDL_RenderCopy(renderer, im_select_texture, NULL, &button_rect);

    button_rect = (SDL_Rect){win_w-90, 0, 90, 90};
    SDL_RenderCopy(renderer, im_select_texture, NULL, &button_rect);
    button_rect = (SDL_Rect){win_w-180, 0, 90, 90};
    SDL_RenderCopy(renderer, im_select_texture, NULL, &button_rect);


    button_rect = (SDL_Rect){win_w-90, win_h-90, 90, 90};
    SDL_RenderCopy(renderer, im_select_texture, NULL, &button_rect);
    if (android_mode == 1)
    {
        sprintf(text, "place");
        write_text(win_w-80, win_h-80, text, (SDL_Color){255,255,255,255}, 28, window, renderer);
    }
    if (android_mode == 3)
    {
        sprintf(text, "break");
        write_text(win_w-80, win_h-80, text, (SDL_Color){255,255,255,255}, 28, window, renderer);
    }
#endif
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

int in_between_normal(int a, int min, int max)
{
    if (a >= min && a <= max)
        return 1;
    return 0;
}

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
    SDL_Log("____________ PLAYER_CHECK_TILE_COLLISION");
    if (world->var)
    {
        SDL_Log("_________ HAS WORLD");
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
        SDL_Log("________________________No world!\n");
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
        SDL_Log("No world!\n");
    }
    return NULL;
}

void update(const Uint8 * keys)
{
    int x,y;
    SDL_GetMouseState(&x, &y);
    int win_h, win_w;
    SDL_GetWindowSize(window, &win_w, &win_h);
    #ifndef ANDROID
    if (HOLD)
    {
        char can_place = 1;
        struct List * current = world;
        struct Tile * _current_tile;
        for (;;)
        {
            if (!current)
                break;
            _current_tile=(struct Tile *)(current->var);
            if (!_current_tile)
                continue;

            if (current->next)
            {
                if (_current_tile->x == (x+player.x-(win_w/2-32))/64 && _current_tile->y == (y+player.y-(win_h/2-32))/64 && _current_tile->id == tiles_order[current_tile])
                {
                    can_place = 0;
                }
                current=current->next;
            }
            else
                break;
        }
        if (can_place)
        {
            play_sound(tiles_sounds[tiles_order[current_tile]]);
            List_append(world,
                        Tile_create(
                                (x+player.x-(win_w/2-32))/64,
                                (y+player.y-(win_h/2-32))/64,
                            tiles_order[current_tile]));
        }
    }
#endif
    if (RHOLD)
    {
        struct List * current = world;
        struct Tile * current_tile;

        while (current->next)
            current = current->next;
        for (;;) 
        {
            current_tile = ((struct Tile *)(current->var));

            if ((current_tile->x == (x+player.x-(win_w/2-32))/64) && (current_tile->y == (y+player.y-(win_h/2-32))/64))
            {
                play_sound(tiles_sounds[current_tile->id]);
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
                break;
            }

            if (current->previous)
                current=current->previous;
            else
                break;
        }
    }

    SDL_Log("-------------- SPEED");
    if (keys[SDL_SCANCODE_LSHIFT])
        player.speed = (int)((5*60)/fps);
    else if (keys[SDL_SCANCODE_LCTRL])
        player.speed = (int)((20*60)/fps);
    else
        player.speed = (int)((10*60)/fps);

    #ifdef ANDROID
    if (HOLD)
    {
        if (in_between_normal(x, 10, 10+90) && in_between_normal(y, win_h-190, win_h-190+90))
        {
            player.x-=player.speed;
            player.going_right=0;
        }
        else if (in_between_normal(x, 190, 190+90) && in_between_normal(y, win_h-190, win_h-190+90))
        {
            player.x+=player.speed;
            player.going_right=1;
        }
        else if (in_between_normal(x, 100, 100+90) && in_between_normal(y, win_h-100, win_h-100+90))
            player.y+=player.speed;
        else if (in_between_normal(x, 100, 100+90) && in_between_normal(y, win_h-280, win_h-280+90))
            player.y-=player.speed;

        else if (!(
            (in_between_normal(x, win_w-90, win_w) && in_between_normal(y, 0, 90)) || 
            (in_between_normal(x, win_w-180, win_w-90) && in_between_normal(y, 0, 90)) ||
            (in_between_normal(x, win_w-90, win_w) && in_between_normal(y, win_h-90, win_h))
        ))
        {
            if (android_mode == 1)
            {
                char can_place = 1;
                struct List * current = world;
                struct Tile * _current_tile;
                for (;;)
                {
                    if (!current)
                        break;
                    _current_tile=(struct Tile *)(current->var);
                    if (!_current_tile)
                        continue;

                    if (current->next)
                    {
                        if (_current_tile->x == (x+player.x-(win_w/2-32))/64 && _current_tile->y == (y+player.y-(win_h/2-32))/64 && _current_tile->id == current_tile)
                            can_place = 0;
                        current=current->next;
                    }
                    else
                        break;
                }
                if (can_place)
                {
                play_sound(tiles_sounds[tiles_order[current_tile]]);
                List_append(world,
                            Tile_create(
                                    (x+player.x-(win_w/2-32))/64,
                                    (y+player.y-(win_h/2-32))/64,
                                tiles_order[current_tile]));
                }
            }
            if (android_mode == 3)
            {
                struct List * current = world;
                struct Tile * current_tile;

                while (current->next)
                    current = current->next;
                for (;;) 
                {
                    current_tile = ((struct Tile *)(current->var));

                    if ((current_tile->x == (x+player.x-(win_w/2-32))/64) && (current_tile->y == (y+player.y-(win_h/2-32))/64))
                    {
                        play_sound(tiles_sounds[current_tile->id]);
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
                        break;
                    }

                    if (current->previous)
                        current=current->previous;
                    else
                        break;
                }
            }
            save();
        }
    }
    #endif

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

    SDL_Log("-------------- MOVE DONE");

    SDL_Log("-------------- COLL CHECK");
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
    SDL_Log("-------------- COLL DONE");
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
    init_sounds();
    playerr_texture = load_texture("textures/playerr.png");
    playerl_texture = load_texture("textures/playerl.png");
    tile_sheet = load_texture("textures/terrain.png");
    im_select_texture = load_texture("textures/select.png");

    Uint32 lt = 0;
    Uint32 ct = 0;
    int fc = 0;

    SDL_Log("-------------- MAIN LOOP");

    for (;;)
    {
        ct = SDL_GetTicks();
        fc++;

        // Keyboard handling by events
        SDL_Log("-------------- KEYBOARD HANDLE");
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
                        save();
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
                    case SDLK_TAB:
                        current_tile++;
                        if (current_tile==TILE_max)
                            current_tile = 0;
                        break;
                }
            }
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == 3)
                RHOLD=0;
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == 1)
                HOLD=0;
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);
                int win_h, win_w;
                SDL_GetWindowSize(window, &win_w, &win_h);
                if (event.button.button == 1)
                {
                    #ifdef ANDROID
                    if (in_between_normal(x, win_w-90, win_w) && in_between_normal(y, 0, 90))
                    {
                        current_tile++;
                        if (current_tile==TILE_max)
                            current_tile = 0;
                    }
                    if (in_between_normal(x, win_w-180, win_w-90) && in_between_normal(y, 0, 90))
                    {
                        current_tile--;
                        if (current_tile==-1)
                            current_tile=TILE_max-1;
                    }
                    if (in_between_normal(x, win_w-90, win_w) && in_between_normal(y, win_h-90, win_h))
                    {
                        if (android_mode == 1)
                            android_mode = 3;
                        else if (android_mode == 3)
                            android_mode = 1;
                    }
                    #endif
                    HOLD=1;
                }
                if (event.button.button == 3)
                {
                    RHOLD=1;
                }
            }
        }

        SDL_Log("-------------- CLEAR");
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Log("-------------- FPS");
        if (ct > lt + 1000) {
            fps = fc;
            fc = 0;
            lt = ct;
        }
        SDL_Log("-------------- KEY BY STATE");
        // Keyboard handling by states
        const Uint8 * keys = SDL_GetKeyboardState(NULL);
        SDL_Log("-------------- UPDATE");
        update(keys);
        SDL_Log("-------------- DRAW");
        draw();
        SDL_Log("-------------- END MAIN LOOP");

        SDL_RenderPresent(renderer);
    }
}

        
