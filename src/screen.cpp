//
// Created by arapo on 10.09.2024.
//

#include "screen.h"
void Screen::setPixel(uint8_t x, uint8_t y, uint8_t bit, uint8_t* Vf){
    if (y >= SCREEN_HEIGHT) {
        return; 
    }
    
    x = x % SCREEN_WIDTH;
    
    auto before = video[y * SCREEN_WIDTH + x];
    video[y * SCREEN_WIDTH + x] = video[y * SCREEN_WIDTH + x] xor bit;
    if(before == 1 and bit == 1){
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

ErrorOr<OwnPtr<Screen>> Screen::try_create(int screen_size_factor)
{
    auto screen = TRY(adopt_nonnull_own_or_enomem(new (nothrow) Screen()));
    TRY(screen->initialize(screen_size_factor));
    return screen;
}

ErrorOr<void> Screen::initialize(int screen_size_factor)
{
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        return Error::from_string_literal("SDL failed to initialize");
    }
    window = SDL_CreateWindow("c8 Serenity",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH * screen_size_factor, SCREEN_HEIGHT * screen_size_factor,0);

    if (window == NULL) {
        cleanup_resources();
        return Error::from_string_literal("Failed to create SDL window");
    }

    screen_small = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT,32,0,0,0,0);
    if (!screen_small) {
        cleanup_resources();
        return Error::from_string_literal("Failed to create small surface");
    }

    screen_big = SDL_CreateRGBSurface(0, SCREEN_WIDTH * screen_size_factor, SCREEN_HEIGHT * screen_size_factor,32,0,0,0,0);
    if (!screen_big) {
        cleanup_resources();
        return Error::from_string_literal("Failed to create big surface");
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_TARGETTEXTURE );
    if( renderer == NULL )
    {
        cleanup_resources();
        return Error::from_string_literal("Failed to create SDL renderer");
    }

    texture =  SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH * screen_size_factor, SCREEN_HEIGHT * screen_size_factor);
    if (!texture) {
        cleanup_resources();
        return Error::from_string_literal("Failed to create SDL texture");
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 44100;     // Sample rate
    want.format = AUDIO_F32SYS; // 32-bit float samples
    want.channels = 1;     // Mono
    want.samples = 256;    // Much smaller buffer for lower latency (~5.8ms instead of ~23ms)
    want.callback = audio_callback;
    want.userdata = this;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (audio_device == 0) {
        fprintf(stderr, "SDL audio failed to initialize: %s\n", SDL_GetError());
    } else {
        SDL_PauseAudioDevice(audio_device, 0);
    }
    
    return {};
}

void Screen::cleanup_resources()
{
    // Clean up audio resources first
    if(audio_device != 0){
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
    
    // Clean up SDL resources in reverse order of creation
    if(texture){
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    if(renderer){
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if(screen_big){
        SDL_FreeSurface(screen_big);
        screen_big = nullptr;
    }

    if(screen_small){
        SDL_FreeSurface(screen_small);
        screen_small = nullptr;
    }

    if(window){
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

Screen::~Screen()
{
    cleanup_resources();
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

void Screen::audio_callback(void* userdata, Uint8* stream, int len)
{
    Screen* screen = static_cast<Screen*>(userdata);
    float* fstream = reinterpret_cast<float*>(stream);
    int samples = len / sizeof(float);
    
    static float phase = 0.0f;
    const float frequency = 440.0f; // A4 note
    const float sample_rate = 44100.0f;
    const float amplitude = 0.1f; // Quiet beep
    
    if (screen->is_beeping) {
        for (int i = 0; i < samples; i++) {
            fstream[i] = (phase < 0.5f) ? amplitude : -amplitude;
            phase += frequency / sample_rate;
            if (phase >= 1.0f) phase -= 1.0f;
        }
    } else {
        for (int i = 0; i < samples; i++) {
            fstream[i] = 0.0f;
        }
        phase = 0.0f; 
    }
}

void Screen::start_beep()
{
    if (audio_device != 0 && !is_beeping) {
        is_beeping = true;
    }
}

void Screen::stop_beep()
{
    if (audio_device != 0 && is_beeping) {
        is_beeping = false;
    }
}
