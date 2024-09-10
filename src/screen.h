//
// Created by arapo on 10.09.2024.
//

#ifndef SERENITYOS_SCREEN_H
#define SERENITYOS_SCREEN_H

#include "AK/Types.h"
#include "AK/OwnPtr.h"

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

class Screen {
public:

    Screen() = default;
    ~Screen() = default;
    void setPixel(uint8_t, uint8_t,uint8_t, uint8_t*);
    void clear();
    void print();
private:
    uint8_t video[SCREEN_WIDTH * SCREEN_HEIGHT]{};
};

#endif // SERENITYOS_SCREEN_H
