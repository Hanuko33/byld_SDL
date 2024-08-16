#ifndef WORLD_H
#define WORLD_H
#include "sound.h"
enum tiles
{
    TILE_blue_wall,
    TILE_green_wall,
    TILE_orange_wall,
    TILE_pink_wall,
    TILE_red_wall,
    TILE_violet_wall,
    TILE_yellow_wall,
    TILE_flower,
    TILE_dirt,
    TILE_water,
    TILE_gray_wall,
    TILE_white_wall,
    Tile_black_wall,
    TILE_max
};

static char tile_solid[] = {
    1, // TILE_blue_wall
    1, // TILE_green_wall
    1, // TILE_orange_wall
    1, // TILE_pink_wall
    1, // TILE_red_wall
    1, // TILE_violet_wall
    1, // TILE_yellow_wall
    0, // TILE_flower
    0, // TILE_dirt
    0, // TILE_water
    1, // TILE_gray_wall
    1, // TILE_white_wall
    1, // TILE_black_wall
};

static enum sounds_enum tiles_sounds[] = {
    SOUND_stone, // TILE_blue_wall
    SOUND_stone, // TILE_green_wall
    SOUND_stone, // TILE_orange_wall
    SOUND_stone, // TILE_pink_wall
    SOUND_stone, // TILE_red_wall
    SOUND_stone, // TILE_violet_wall
    SOUND_stone, // TILE_yellow_wall
    SOUND_dirt,  // TILE_flower
    SOUND_dirt,  // TILE_dirt
    SOUND_max,   // TILE_water
    SOUND_stone, // TILE_gray_wall
    SOUND_stone, // TILE_white_wall
    SOUND_stone  // TILE_black_wall
};

#endif
