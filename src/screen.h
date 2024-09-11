//
// Created by arapo on 10.09.2024.
//

#ifndef SERENITYOS_SCREEN_H
#define SERENITYOS_SCREEN_H

#include "AK/Types.h"
#include "AK/OwnPtr.h"
#include <SDL2/SDL.h>


const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

class Screen {
public:
    Screen(int screen_size_factor);
    ~Screen();
    void setPixel(uint8_t, uint8_t,uint8_t, uint8_t*);
    void clear();
    void print();
    void sdl_render();

private:
    uint32_t video[SCREEN_WIDTH * SCREEN_HEIGHT]{};
    SDL_Window *window = NULL;
    SDL_Surface *screen_small = NULL;
    SDL_Surface *screen_big = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
};

#endif // SERENITYOS_SCREEN_H
