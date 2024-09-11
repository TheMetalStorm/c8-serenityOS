//
// Created by arapo on 10.09.2024.
//
#include "Chip8.h"

unsigned int const FONTSET_SIZE = 80;
unsigned int const VF = 15;
bool KEYS[322]; // 322 is the number of SDLK_DOWN events

enum chip8_sdl_key_translation {
    KEYPAD_ONE = SDLK_1,
    KEYPAD_TWO = SDLK_2,
    KEYPAD_THREE = SDLK_3,
    KEYPAD_C = SDLK_4,

    KEYPAD_FOUR = SDLK_q,
    KEYPAD_FIVE = SDLK_w,
    KEYPAD_SIX = SDLK_e,
    KEYPAD_D = SDLK_r,

    KEYPAD_SEVEN = SDLK_a,
    KEYPAD_EIGHT = SDLK_s,
    KEYPAD_NINE = SDLK_d,
    KEYPAD_E = SDLK_f,

    KEYPAD_A = SDLK_y,
    KEYPAD_ZERO = SDLK_x,
    KEYPAD_B = SDLK_c,
    KEYPAD_F = SDLK_v,
};
enum chip8_key {
    ONE,
    TWO,
    THREE,
    LETTER_C,
    FOUR,
    FIVE,
    SIX,
    LETTER_D,
    SEVEN,
    EIGHT,
    NINE,
    LETTER_E,
    LETTER_A,
    ZERO,
    LETTER_B,
    LETTER_F
};

uint8_t fontset[FONTSET_SIZE] = {
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
    for (size_t i = 0; i < rom_content.bytes().size(); i++) {
        memory[rom_start + i] = rom_content.bytes()[i];
    }

    return {};
}

ErrorOr<void> Chip8::run()
{
    while (program_counter <= 0xFFF) {
        handle_input();
        if (!is_running)
            break;
        // instruction fetch
        auto next_instruction = get_next_instruction();
        //        printf("%x\n", next_instruction);
        // instruction decode
        TRY(decode_and_execute(next_instruction));
    }
    return {};
}

uint16_t Chip8::get_next_instruction()
{
    uint16_t const instruction = memory[program_counter++] << 8;
    return instruction | memory[program_counter++];
}

Chip8::Chip8()
{
    program_counter = rom_start;

    for (uint32_t i = 0; i < FONTSET_SIZE; ++i) {
        memory[font_start + i] = fontset[i];
    }
    for (int i = 0; i < 322; i++) { // init them all to false
        KEYS[i] = false;
    }

    for (int i = 0; i < 16; i++) { // init them all to false
        keypad[i] = false;
    }
}
ErrorOr<void> Chip8::decode_and_execute(uint16_t instruction)
{
    uint16_t kk, Vx, Vy, nnn, n, random_number;

    switch (instruction) {
    case 0x00E0: // CLS
        screen->clear();
        break;
    case 0x00EE: // RET
        program_counter = TRY(stack.pop());
        break;
    case 0x1000 ... 0x1FFF: // 1nnn JP addr nnn
        program_counter = get_nnn(instruction);
        break;
    case 0x2000 ... 0x2FFF: // 2nnn CALL addr nnn
        TRY(stack.push(program_counter));
        program_counter = get_nnn(instruction);
        break;
    case 0x3000 ... 0x3FFF: // 3xkk Skip next instruction if Vx = kk
        Vx = get_Vx(instruction);
        kk = get_kk(instruction);
        if (registers[Vx] == kk) {
            program_counter += 2;
        }
        break;
    case 0x4000 ... 0x4FFF: // 4xkk Skip next instruction if Vx != kk
        Vx = get_Vx(instruction);
        kk = get_kk(instruction);
        if (registers[Vx] != kk) {
            program_counter += 2;
        }
        break;
    case 0x5000 ... 0x5FFF: // 5xy0 - SE Vx, Vy Skip next instruction if Vx = Vy.
        Vx = get_Vx(instruction);
        Vy = get_Vy(instruction);
        if (registers[Vx] == registers[Vy]) {
            program_counter += 2;
        }
        break;
    case 0x6000 ... 0x6FFF: // 6xkk - LD Vx, byte Set Vx = kk.
        Vx = get_Vx(instruction);
        kk = get_kk(instruction);
        registers[Vx] = kk;
        break;
    case 0x7000 ... 0x7FFF: // 7xkk - ADD Vx, byteSet Vx = Vx + kk.
        Vx = get_Vx(instruction);
        kk = get_kk(instruction);
        registers[Vx] += kk;
        break;
    case 0x8000 ... 0x8FFF:
        handle_8xxx(instruction);
        break;
    case 0x9000 ... 0x9FFF: // SNE Vx, Vy Skip next instruction if Vx != Vy.
        Vx = registers[get_Vx(instruction)];
        Vy = registers[get_Vy(instruction)];
        if (Vx != Vy) {
            program_counter += 2;
        }
        break;
    case 0xA000 ... 0xAFFF: // Annn - LD I, addr Set I = nnn.
        nnn = get_nnn(instruction);
        index_register = nnn;
        break;
    case 0xB000 ... 0xBFFF: // JP V0, addr Jump to location nnn + V0.
        nnn = get_nnn(instruction);
        program_counter = nnn + registers[0];
        break;
    case 0xC000 ... 0xCFFF: // Cxkk - RND Vx, byte - Set Vx = random byte AND kk.
        Vx = get_Vx(instruction);
        kk = get_kk(instruction);
        random_number = get_random_uniform(255);
        registers[Vx] = random_number & kk;
        break;
    case 0xD000 ... 0xDFFF: // Dxyn - DRW Vx, Vy, nibble Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        Vx = registers[get_Vx(instruction)];
        Vy = registers[get_Vy(instruction)];
        n = get_n(instruction);
        registers[0xF] = 0;

        for (int y = 0; y < n; y++) {
            uint8_t const sprite_line = memory[index_register + y];
            for (int x = 0; x < 8; x++) {
                uint8_t const bit = get_bit(sprite_line, 7 - x);
                auto* Vf = &registers[0xF];

                screen->setPixel(x + Vx, y + Vy, bit, Vf);
            }
        }
        //            screen->print();
        break;
    default:
        if (instruction != 0)
            printf("UNKNOWN\n");
    }

    return {};
}

uint16_t Chip8::get_bit(uint16_t byte, uint8_t bit)
{
    return (byte >> bit) & 1;
}
uint16_t Chip8::get_kk(uint16_t instruction)
{
    return (instruction & 0x00FF);
}
uint16_t Chip8::get_Vx(uint16_t instruction)
{
    return (instruction & 0x0F00) >> 8;
}
uint16_t Chip8::get_Vy(uint16_t instruction)
{
    return (instruction & 0x00F0) >> 4;
}
uint16_t Chip8::get_nnn(uint16_t instruction)
{
    return instruction & 0xFFF;
}
uint16_t Chip8::get_n(uint16_t instruction)
{
    return instruction & 0x000F;
}
void Chip8::handle_8xxx(uint16_t instruction)
{
    uint16_t const n = get_n(instruction);
    uint16_t const Vx = get_Vx(instruction);
    uint16_t const Vy = get_Vy(instruction);
    uint16_t result;
    switch (n) {
    case 0x0: // 8xy0 - LD Vx, VySet Vx = Vy.
        registers[Vx] = registers[Vy];
        break;
    case 0x1: // 8xy1 - OR Vx, Vy Set Vx = Vx OR Vy.
        registers[Vx] = registers[Vx] | registers[Vy];
        break;
    case 0x2: // 8xy2 - And Vx, Vy Set Vx = Vx AND Vy.
        registers[Vx] = registers[Vx] & registers[Vy];
        break;
    case 0x3: // 8xy3 - Xor Vx, Vy Set Vx = Vx Xor Vy.
        registers[Vx] = registers[Vx] xor registers[Vy];
        break;
    case 0x4: // 8xy4 - Set Vx = Vx + Vy, set VF = carry.
        result = (uint16_t)registers[Vx] + (uint16_t)registers[Vy];
        if (((result & 0xFF00) >> 8) > 0) {
            registers[VF] = 1;
        } else {
            registers[VF] = 0;
        }
        registers[Vx] = (result & 0x00FF);
        break;
    case 0x5: // SUB Vx, Vy Set Vx = Vx - Vy, set VF = NOT borrow.
        if (registers[Vx] > registers[Vy]) {
            registers[VF] = 1;
        } else {
            registers[VF] = 0;
        }
        registers[Vx] -= registers[Vy];
        break;
    case 0x6: // 8xy6 - SHR Vx {, Vy}  VF = LSB Vx. Set Vx = Vx SHR 1.
        registers[VF] = (registers[Vx] & 1);
        registers[Vx] = registers[Vx] >> 1;
        break;
    case 0x7: // 8xy7 - SUBN Vx, Vy Set Vx = Vy - Vx, set VF = NOT borrow.
        if (registers[Vy] > registers[Vx]) {
            registers[VF] = 1;
        } else {
            registers[VF] = 0;
        }
        registers[Vx] = registers[Vy] - registers[Vx];
        break;
    case 0xE: // 8xyE - SHL Vx {, Vy} VF = MSB Vx.  Set Vx = Vx SHL 1.
        registers[VF] = (registers[Vx] & 0b10000000) >> 7;
        registers[Vx] = registers[Vx] << 1;
        break;
    default:
        outln("UNSUPPORTED INSTRUCTION OR OTHER DATA IN ROM: {}", instruction);
    }
}
void Chip8::handle_input()
{
    //TODO: consider this (consider this!) (its a reference to "losing my religion")
    //https://retrocomputing.stackexchange.com/questions/358/how-are-held-down-keys-handled-in-chip-8

    //FIXME: crash when i press CTRL while SDL window is in focus
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            return;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                is_running = false;
                return;
            }
            KEYS[event.key.keysym.sym] = true;
            break;
        case SDL_KEYUP:
            KEYS[event.key.keysym.sym] = false;
            break;
        default:
            break;
        }
    }

    keypad[chip8_key::ONE] = KEYS[chip8_sdl_key_translation::KEYPAD_ONE];
    keypad[chip8_key::TWO] = KEYS[chip8_sdl_key_translation::KEYPAD_TWO];
    keypad[chip8_key::THREE] = KEYS[chip8_sdl_key_translation::KEYPAD_THREE];
    keypad[chip8_key::LETTER_C] = KEYS[chip8_sdl_key_translation::KEYPAD_C];

    keypad[chip8_key::FOUR] = KEYS[chip8_sdl_key_translation::KEYPAD_FOUR];
    keypad[chip8_key::FIVE] = KEYS[chip8_sdl_key_translation::KEYPAD_FIVE];
    keypad[chip8_key::SIX] = KEYS[chip8_sdl_key_translation::KEYPAD_SIX];
    keypad[chip8_key::LETTER_D] = KEYS[chip8_sdl_key_translation::KEYPAD_D];

    keypad[chip8_key::SEVEN] = KEYS[chip8_sdl_key_translation::KEYPAD_SEVEN];
    keypad[chip8_key::EIGHT] = KEYS[chip8_sdl_key_translation::KEYPAD_EIGHT];
    keypad[chip8_key::NINE] = KEYS[chip8_sdl_key_translation::KEYPAD_NINE];
    keypad[chip8_key::LETTER_E] = KEYS[chip8_sdl_key_translation::KEYPAD_E];

    keypad[chip8_key::LETTER_A] = KEYS[chip8_sdl_key_translation::KEYPAD_A];
    keypad[chip8_key::ZERO] = KEYS[chip8_sdl_key_translation::KEYPAD_ZERO];
    keypad[chip8_key::LETTER_B] = KEYS[chip8_sdl_key_translation::KEYPAD_B];
    keypad[chip8_key::LETTER_F] = KEYS[chip8_sdl_key_translation::KEYPAD_F];
}
