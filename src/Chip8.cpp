//
// Created by arapo on 10.09.2024.
//
#include "Chip8.h"
#include "AK/Random.h"
#include "LibCore/File.h"
#include <stdio.h>

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
//bool Chip8::is_running()
//{
//    return true;
//}
ErrorOr<void> Chip8::run()
{
    while(program_counter < 0xFFF){
        //instruction fetch
        auto next_instruction = get_next_instruction();
        //instruction decode
        TRY(decode_and_execute(next_instruction));
    }
    return {};
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
ErrorOr<void> Chip8::decode_and_execute(uint16_t instruction){
    uint8_t kk, Vx, Vy;//, n, nnn;
//    printf("INSTRUCTION %02x %02x ...\n", instruction & 0xFF,  (instruction>>8) & 0xFF);
    printf("INSTRUCTION %04x \n",instruction);
    switch (instruction) {
        case 0x00E0: //CLS
            memset(video, 0, sizeof(video));
            break;
        case 0x00EE: //RET
            program_counter = TRY(stack.pop());
            break;
        case 0x1000 ... 0x1FFF: //1nnn JP addr nnn
            program_counter = get_nnn(instruction);
            break;
        case 0x2000 ... 0x2FFF: //2nnn CALL addr nnn
            outln("CALL {}", program_counter);
            TRY(stack.push(program_counter));
            program_counter = get_nnn(instruction);
            break;
        case 0x3000 ... 0x3FFF: // 3xkk Skip next instruction if Vx = kk
            Vx = get_Vx(instruction);
            kk = get_kk(instruction);
            if(registers[Vx] == kk){
                program_counter+=2;
            }
            break;
        case 0x4000 ... 0x4FFF: // 4xkk Skip next instruction if Vx != kk
            Vx = get_Vx(instruction);
            kk = get_kk(instruction);
            if(registers[Vx] != kk){
                program_counter+=2;
            }
            break;
        case 0x5000 ... 0x5FFF: // 5xy0 - SE Vx, Vy Skip next instruction if Vx = Vy.
            Vx = get_Vx(instruction);
            Vy = get_Vy(instruction);
            if(registers[Vx] == registers[Vy]){
                program_counter+=2;
            }
            break;
        case 0x6000 ... 0x6FFF: // 6xkk - LD Vx, byte Set Vx = kk.
            Vx = get_Vx(instruction);
            kk = get_kk(instruction);
            registers[Vx] = kk;
            break;
        case 0x7000 ... 0x7FFF: //7xkk - ADD Vx, byteSet Vx = Vx + kk.
            Vx = get_Vx(instruction);
            kk = get_kk(instruction);
            registers[Vx] += kk;
            break;
        case 0x8000 ... 0x8FFF:
            handle_8xxx(instruction);
            break;
        default:
            if(instruction != 0)
                printf("UNKNOWN\n");

    }

    return {};


}


uint8_t Chip8::get_kk(uint8_t instruction) {
    return (instruction & 0x00FF);
}
uint8_t Chip8::get_Vx(uint8_t instruction) {
    return (instruction & 0x0F00) >> 8;
}
uint8_t Chip8::get_Vy(uint8_t instruction) {
    return (instruction & 0x00F0) >> 4;
}
uint8_t Chip8::get_nnn(uint8_t instruction) {
    return instruction & 0x0FFF;
}
uint8_t Chip8::get_n(uint8_t instruction) {
    return instruction & 0x000F;
}
void Chip8::handle_8xxx(uint8_t instruction)
{
    uint8_t n = get_n(instruction);
    uint8_t Vx = get_Vx(instruction);
    uint8_t Vy= get_Vy(instruction);

    switch (n) {
        case 0x0: //8xy0 - LD Vx, VySet Vx = Vy.
                registers[Vx] = registers[Vy];
                break;

        default:
                outln("UNSUPPORTED INSTRUCTION OR OTHER DATA IN ROM: {}", instruction);
    }
}
