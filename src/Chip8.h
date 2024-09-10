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
    uint8_t memory[4096]= {};
    uint8_t registers[16] = {};
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
    ErrorOr<void> run();
    uint16_t get_next_instruction();
    ErrorOr<void> decode_and_execute(uint16_t
            instruction);

    // nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
    // n or nibble - A 4-bit value, the lowest 4 bits of the instruction
    // x - A 4-bit value, the lower 4 bits of the high byte of the instruction
    // y - A 4-bit value, the upper 4 bits of the low byte of the instruction
    // kk or byte - An 8-bit value, the lowest 8 bits of the instruction
    uint8_t get_kk(uint8_t
            instruction);
    uint8_t get_Vy(uint8_t
            instruction);
    uint8_t get_Vx(uint8_t
            instruction);
    uint8_t get_nnn(uint8_t
            instruction);
    uint8_t get_n(uint8_t
            instruction);
    void handle_8xxx(uint8_t instruction);
};

#endif // SERENITY_CHIP8_H
