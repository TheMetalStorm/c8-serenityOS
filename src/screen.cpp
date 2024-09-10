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
