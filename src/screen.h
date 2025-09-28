//
// Created by arapo on 10.09.2024.
//

#ifndef SERENITYOS_SCREEN_H
#define SERENITYOS_SCREEN_H

#include "AK/Types.h"
#include "AK/OwnPtr.h"
#include "AK/Error.h"
#include <SDL2/SDL.h>


const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

class Screen {
public:
    static ErrorOr<OwnPtr<Screen>> try_create(int screen_size_factor);
    ~Screen();
    void setPixel(uint8_t, uint8_t,uint8_t, uint8_t*);
    void clear();
    void print();
    void sdl_render();
    void start_beep();
    void stop_beep();

private:
    Screen() = default;
    ErrorOr<void> initialize(int screen_size_factor);
    void cleanup_resources();
    uint32_t video[SCREEN_WIDTH * SCREEN_HEIGHT]{};
    SDL_Window *window = NULL;
    SDL_Surface *screen_small = NULL;
    SDL_Surface *screen_big = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    
    // Audio members
    SDL_AudioDeviceID audio_device = 0;
    bool is_beeping = false;
    static void audio_callback(void* userdata, Uint8* stream, int len);
};

#endif // SERENITYOS_SCREEN_H
