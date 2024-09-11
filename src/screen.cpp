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
Screen::Screen(int screen_size_factor)
{
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());



        //TODO error handling
        //return 1;
    }
    window = SDL_CreateWindow("SDL Example",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH * screen_size_factor, SCREEN_HEIGHT * screen_size_factor,0);

    if (window == NULL) {
        fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
        //TODO error handling
        //return 1;
    }

    screen_small = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT,32,0,0,0,0);
    screen_big = SDL_CreateRGBSurface(0, SCREEN_WIDTH * screen_size_factor, SCREEN_HEIGHT * screen_size_factor,32,0,0,0,0);

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_TARGETTEXTURE );
    if( renderer == NULL )
    {
        fprintf( stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        //TODO error handling
        //return 1;
    }

    texture =  SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH * screen_size_factor, SCREEN_HEIGHT * screen_size_factor);

}

Screen::~Screen()
{
    if(window){
        SDL_DestroyWindow(window);
    }

    if(renderer){
        SDL_DestroyRenderer(renderer);
    }

    if(texture){
        SDL_DestroyTexture(texture);
    }

    SDL_Quit();
}

//yeeted from jborza.com/post/2020-12-07-chip-8/
void Screen::sdl_render()
{
    SDL_LockSurface(screen_small);
    uint32_t *pixels = (uint32_t *)screen_small->pixels;
    for (int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++)
    {
        pixels[i] = video[i] == 0 ? 0 : 0xFFFFFFFF;
    }
    SDL_UnlockSurface(screen_small);

    SDL_BlitScaled(screen_small, NULL, screen_big, NULL);
    SDL_UpdateTexture(texture, NULL, screen_big->pixels, screen_big->pitch);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
