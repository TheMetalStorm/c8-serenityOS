//
// Created by arapo on 10.09.2024.
//
#include "Chip8.h"

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

ErrorOr<void> Chip8::run()
{
    while(program_counter < 0xFFF){
        //instruction fetch
        auto next_instruction = get_next_instruction();
        printf("%x\n", next_instruction);
        //instruction decode
        TRY(decode_and_execute(next_instruction));


        (void)getchar();
    }
    return {};
}

uint16_t Chip8::get_next_instruction(){
    uint16_t const instruction = memory[program_counter++] << 8;
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
    uint16_t kk, Vx, Vy, nnn, n;

    switch (instruction) {
        case 0x00E0: //CLS
            screen.clear();
            break;
        case 0x00EE: //RET
            program_counter = TRY(stack.pop());
            break;
        case 0x1000 ... 0x1FFF: //1nnn JP addr nnn
            program_counter = get_nnn(instruction);
            break;
        case 0x2000 ... 0x2FFF: //2nnn CALL addr nnn
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
            printf("kk = %d\n", kk);
            registers[Vx] += kk;
            printf("v0 = %d\n", registers[Vx]);

            break;
        case 0x8000 ... 0x8FFF:
            handle_8xxx(instruction);
            break;
        case 0xA000 ... 0xAFFF: //Annn - LD I, addr Set I = nnn.
            nnn = get_nnn(instruction);
            index_register = nnn;
            break;
        case 0xD000 ... 0xDFFF: //Dxyn - DRW Vx, Vy, nibble Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            Vx = registers[get_Vx(instruction)];
            Vy = registers[get_Vy(instruction)];

            n = get_n(instruction);


            registers[0xF] = 0;

            for(int y = 0; y< n; y++){
                uint8_t const sprite_line = memory[index_register  + y];
                for(int x = 0; x< 8; x++){
                    uint8_t const bit = get_bit(sprite_line, 7-x);
                    auto *Vf = &registers[0xF];
                    screen.setPixel(x + Vx ,y + Vy , bit, Vf);
                }
            }
            screen.print();
            break;
        default:
            if(instruction != 0)
                printf("UNKNOWN\n");

    }
    return {};
}


uint16_t Chip8::get_bit(uint16_t byte, uint8_t bit){
    return (byte >> bit) & 1;
}
uint16_t Chip8::get_kk(uint16_t instruction) {
    return (instruction & 0x00FF);
}
uint16_t Chip8::get_Vx(uint16_t instruction) {
    return (instruction & 0x0F00) >> 8;
}
uint16_t Chip8::get_Vy(uint16_t instruction) {
    return (instruction & 0x00F0) >> 4;
}
uint16_t Chip8::get_nnn(uint16_t instruction) {
    return instruction & 0xFFF;
}
uint16_t Chip8::get_n(uint16_t instruction) {
    return instruction & 0x000F;
}
void Chip8::handle_8xxx(uint16_t instruction)
{
    uint16_t const n = get_n(instruction);
    uint16_t  const Vx = get_Vx(instruction);
    uint16_t const  Vy= get_Vy(instruction);

    switch (n) {
        case 0x0: //8xy0 - LD Vx, VySet Vx = Vy.
                registers[Vx] = registers[Vy];
                break;

        default:
                outln("UNSUPPORTED INSTRUCTION OR OTHER DATA IN ROM: {}", instruction);
    }
}
