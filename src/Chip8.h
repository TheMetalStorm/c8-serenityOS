//
// Created by arapo on 10.09.2024.
//
#include "AK/Format.h"
#include "Userland/Applications/c8/src/stack.h"
#ifndef SERENITY_CHIP8_H
#define SERENITY_CHIP8_H



class Chip8{
public:
    Chip8();
    const uint16_t font_start = 0x050;
    const uint16_t rom_start = 0x200;
    const uint16_t rom_end = 0xFFF;
    uint8_t memory[4096];
    uint8_t registers[16];
    uint16_t index_register{};
    uint16_t program_counter{};
//    uint8_t stack_pointer{};
    uint8_t delay_timer{};
    uint8_t sound_timer{};
    Stack stack = Stack(16);
    uint8_t keypad[16]{};
    uint32_t video[64 * 32]{};
    uint16_t opcode{};

    ErrorOr<void> read_rom(StringView);
    bool is_running();
    void run();
    uint16_t get_next_instruction();
    void decode_and_execute(uint16_t
            instruction);
};

#endif // SERENITY_CHIP8_H
