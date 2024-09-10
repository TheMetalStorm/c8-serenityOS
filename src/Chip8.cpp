//
// Created by arapo on 10.09.2024.
//
#include "Chip8.h"
#include "AK/Random.h"
#include "LibCore/File.h"

unsigned int const FONTSET_SIZE = 80;

uint8_t fontset[FONTSET_SIZE] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

ErrorOr<void> Chip8::read_rom(StringView rom_path)
{
    auto rom = TRY(Core::File::open(rom_path, Core::File::OpenMode::Read));
    auto rom_content = TRY(rom->read_until_eof());
    for (size_t i = 0; i < rom_content.bytes().size();i++) {
        memory[rom_start+i] = rom_content.bytes()[i];
    }

    return {};
}
bool Chip8::is_running()
{
    return true;
}
void Chip8::run()
{
    while(is_running()){
        //instruction fetch
        auto next_instruction = get_next_instruction();
        //instruction decode
        decode_and_execute(next_instruction);
    }
}

uint16_t Chip8::get_next_instruction(){
    uint16_t instruction = memory[program_counter++] << 8;
    return instruction | memory[program_counter++] ;
}

Chip8::Chip8()
{
    program_counter = rom_start;

    for (uint32_t i = 0; i < FONTSET_SIZE; ++i){
        memory[font_start+i] = fontset[i];
    }

}
void Chip8::decode_and_execute(uint16_t instruction){
    switch (instruction) {

    }
}
