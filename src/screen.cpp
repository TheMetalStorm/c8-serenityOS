//
// Created by arapo on 10.09.2024.
//

#include "screen.h"
void Screen::setPixel(uint8_t x, uint8_t y, uint8_t bit, uint8_t* Vf){
    auto before = video[y * SCREEN_WIDTH + (x%SCREEN_WIDTH)];
    video[y * SCREEN_WIDTH + (x%SCREEN_WIDTH)] = video[y * SCREEN_WIDTH + (x%SCREEN_WIDTH)] xor bit;
    if(before == 1 and bit == 1){
        //collision
        *Vf = 1;
    }
}

void Screen::print(){
    for (int y2 = 0; y2 <32; y2++)
    {
        for (int x2 = 0; x2 < 64; x2++)
        {
            out("{} ", video[y2 * SCREEN_WIDTH + x2 ]);
        }
        outln();
    }
}
void Screen::clear()
{
    memset(video, 0, sizeof(video));
}

//TODO sth like this?
//        ErrorOr<NonnullOwnPtr<UHCIRootHub>> UHCIRootHub::try_create(NonnullLockRefPtr<UHCIController> uhci_controller)
//        {
//            return adopt_nonnull_own_or_enomem(new (nothrow) UHCIRootHub(move(uhci_controller)));
//        }
Screen::Screen()
{
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());



        //TODO error handling
        //return 1;
    }
    window = SDL_CreateWindow("SDL Example", /* Title of the SDL window */
        SDL_WINDOWPOS_UNDEFINED, /* Position x of the window */
        SDL_WINDOWPOS_UNDEFINED, /* Position y of the window */
        200, /* Width of the window in pixels */
        200, /* Height of the window in pixels */
        0); /* Additional flag(s) */

    if (window == NULL) {
        fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
        //TODO error handling
        //return 1;
    }
}

Screen::~Screen()
{
    if(window){
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}
