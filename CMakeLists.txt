cmake_minimum_required(VERSION 3.10)
project(byld_SDL)
find_package(PkgConfig)
pkg_check_modules(SDL sdl2)
pkg_check_modules(SDL2_IMAGE SDL2_image)
pkg_check_modules(SDL2_TTF SDL2_ttf)
pkg_check_modules(SDL2_MIXER SDL2_mixer)
set(CMAKE_BUILD_TYPE Debug)
add_executable(byld_SDL list/list.c text.c rect.c sound.c main.c)

target_link_libraries(byld_SDL
    SDL2 SDL2_image SDL2_ttf SDL2_mixer
    ${SDL_LDFLAGS} -lm)
