//
// Created by arapo on 10.09.2024.
//
#ifndef SERENITY_CHIP8_H
#define SERENITY_CHIP8_H

#include "AK/Format.h"
#include "AK/Random.h"
#include "AK/Time.h"
#include "AK/Vector.h"
#include "LibCore/File.h"
#include "screen.h"
#include "stack.h"
#include <AK/RefPtr.h>
#include <stdio.h>
#include <SDL2/SDL.h>

class Chip8 {
public:
    Chip8(int screen_size_factor);
    uint16_t const font_start = 0x050;
    uint16_t const rom_start = 0x200;
    uint16_t const rom_end = 0xFFF;
    uint8_t memory[4096] = { 0 };
    uint8_t registers[16] = {};
    uint16_t index_register {};
    uint16_t program_counter {};
    uint8_t delay_timer {};
    uint8_t sound_timer {};
    Stack stack = Stack(16);
    bool keypad[16] {};
    bool is_running = true;
    OwnPtr<Screen> screen;
    uint16_t opcode {};
    int lastKeyPressed = -1;


    ErrorOr<void> read_rom(StringView);
    ErrorOr<void> run();
    uint16_t get_next_instruction();
    ErrorOr<void> decode_and_execute(uint16_t
            instruction);

    // nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
    // n or nibble - A 4-bit value, the lowest 4 bits of the instruction
    // x - A 4-bit value, the lower 4 bits of the high byte of the instruction
    // y - A 4-bit value, the upper 4 bits of the low byte of the instruction
    // kk or byte - An 8-bit value, the lowest 8 bits of the instruction
private:
    static uint16_t get_kk(uint16_t
            instruction);
    static uint16_t get_Vy(uint16_t
            instruction);
    static uint16_t get_Vx(uint16_t
            instruction);
    static uint16_t get_nnn(uint16_t
            instruction);
    static uint16_t get_n(uint16_t
            instruction);
    void handle_8xxx(uint16_t instruction);
    static uint16_t get_bit(uint16_t, uint8_t);
    void handle_input();
    void handle_Exxx(uint16_t
            instruction);
    void handle_Fxxx(uint16_t
            instruction);
    static i64 current_time_microseconds(){
        return UnixDateTime::now().milliseconds_since_epoch() * 1000;


    }
};

#endif // SERENITY_CHIP8_H
