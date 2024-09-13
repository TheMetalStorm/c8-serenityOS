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
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    LETTER_A,
    LETTER_B,
    LETTER_C,
    LETTER_D,
    LETTER_E,
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
//    int currentTime = 0;
    while (program_counter <= 0xFFF) {
        handle_input();
        if (!is_running)
            break;
        // instruction fetch
        auto next_instruction = get_next_instruction();
        // instruction decode / render on draw instrutions
        TRY(decode_and_execute(next_instruction));
//        usleep(1428); //700Hz (Wait 1428 microseconds).
    }
    return {};
}

uint16_t Chip8::get_next_instruction()
{
    uint16_t const instruction = memory[program_counter++] << 8;
    return instruction | memory[program_counter++];
}

Chip8::Chip8(int screen_size_factor)
{
    screen = make<Screen>(screen_size_factor);
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
    printf("%d\n", instruction);
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
        screen->sdl_render();
        break;
    case 0xE000 ... 0xEFFF:
        handle_Exxx(instruction);
        break;
    case 0xF000 ... 0xFFFF:
        handle_Fxxx(instruction);
        break;
    default:
        if (instruction != 0)
            printf("UNKNOWN\n");
    }

    return {};
}

void Chip8::handle_8xxx(uint16_t instruction)
{
    uint16_t const n = get_n(instruction);
    uint16_t const Vx = get_Vx(instruction);
    uint16_t const Vy = get_Vy(instruction);
    uint16_t result;
    bool setVf;
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
        setVf = ((result & 0xFF00) >> 8) > 0;
        registers[Vx] = (result & 0x00FF);
        registers[VF] = static_cast<uint8_t>(setVf);
        break;
    case 0x5: // SUB Vx, Vy Set Vx = Vx - Vy, set VF = NOT borrow.
        setVf = registers[Vx] >= registers[Vy];
        registers[Vx] -= registers[Vy];
        registers[VF] = static_cast<uint8_t>(setVf);
        break;
    case 0x6: // 8xy6 - SHR Vx {, Vy}  VF = LSB Vx. Set Vx = Vx SHR 1.
        setVf = (registers[Vx] & 1);
        registers[Vx] = registers[Vx] >> 1;
        registers[VF] = static_cast<uint8_t>(setVf);
        break;
    case 0x7: // 8xy7 - SUBN Vx, Vy Set Vx = Vy - Vx, set VF = NOT borrow.
        setVf = registers[Vy] >= registers[Vx];
        registers[Vx] = registers[Vy] - registers[Vx];
        registers[VF] = static_cast<uint8_t>(setVf);
        break;
    case 0xE: // 8xyE - SHL Vx {, Vy} VF = MSB Vx.  Set Vx = Vx SHL 1.
        setVf = (registers[Vx] & 0b10000000) >> 7;
        registers[Vx] = registers[Vx] << 1;
        registers[VF] = static_cast<uint8_t>(setVf);
        break;
    default:
        outln("UNSUPPORTED INSTRUCTION OR OTHER DATA IN ROM: {}", instruction);
    }
}

void Chip8::handle_Exxx(uint16_t instruction)
{
    uint16_t Vx;
    switch (instruction) {
    case 0xE09E:
    case 0xE19E:
    case 0xE29E:
    case 0xE39E:
    case 0xE49E:
    case 0xE59E:
    case 0xE69E:
    case 0xE79E:
    case 0xE89E:
    case 0xE99E:
    case 0xEA9E:
    case 0xEB9E:
    case 0xEC9E:
    case 0xED9E:
    case 0xEE9E:
    case 0xEF9E: // Ex9E - SKP Vx - Skip next instruction if key with the value of Vx is pressed.
        Vx = registers[get_Vx(instruction)];
        if (keypad[Vx]) {
            program_counter += 2;
        }
        break;
    case 0xE0A1: // ExA1 - SKNP  Vx - Skip next instruction if key with the value of Vx is not pressed.
    case 0xE1A1:
    case 0xE2A1:
    case 0xE3A1:
    case 0xE4A1:
    case 0xE5A1:
    case 0xE6A1:
    case 0xE7A1:
    case 0xE8A1:
    case 0xE9A1:
    case 0xEAA1:
    case 0xEBA1:
    case 0xECA1:
    case 0xEDA1:
    case 0xEEA1:
    case 0xEFA1:
        Vx = registers[get_Vx(instruction)];
        if (!keypad[Vx]) {
            program_counter += 2;
        }
        break;
    default:
        outln("UNSUPPORTED INSTRUCTION OR OTHER DATA IN ROM: {}", instruction);
    }
}

void Chip8::handle_Fxxx(uint16_t instruction)
{
    uint16_t Vx;
    int foundKey = -1;
    switch (instruction) {
    case 0xF007: // LD Vx, DT - Set Vx = delay timer value.
    case 0xF107:
    case 0xF207:
    case 0xF307:
    case 0xF407:
    case 0xF507:
    case 0xF607:
    case 0xF707:
    case 0xF807:
    case 0xF907:
    case 0xFA07:
    case 0xFB07:
    case 0xFC07:
    case 0xFD07:
    case 0xFE07:
    case 0xFF07:
        Vx = get_Vx(instruction);
        registers[Vx] = delay_timer;
        break;
    case 0xF00A: // Fx0A - LD Vx, K - Wait for a key press, store the value of the key in Vx.
    case 0xF10A:
    case 0xF20A:
    case 0xF30A:
    case 0xF40A:
    case 0xF50A:
    case 0xF60A:
    case 0xF70A:
    case 0xF80A:
    case 0xF90A:
    case 0xFA0A:
    case 0xFB0A:
    case 0xFC0A:
    case 0xFD0A:
    case 0xFE0A:
    case 0xFF0A:
        Vx = get_Vx(instruction);
        for (int i = 0; i < 16; i++) {
            if (keypad[i]) {
                foundKey = i;
                break;
            }
        }

        if (foundKey != -1) {
            registers[Vx] = foundKey;
        } else {
            program_counter -= 2;
        }

        break;
    case 0xF015: // Fx15 - LD DT, Vx Set delay timer = Vx.
    case 0xF115:
    case 0xF215:
    case 0xF315:
    case 0xF415:
    case 0xF515:
    case 0xF615:
    case 0xF715:
    case 0xF815:
    case 0xF915:
    case 0xFA15:
    case 0xFB15:
    case 0xFC15:
    case 0xFD15:
    case 0xFE15:
    case 0xFF15:
        Vx = get_Vx(instruction);
        delay_timer = registers[Vx];
        break;
    case 0xF018: // Fx18 - LD ST, Vx - Set sound timer = Vx.
    case 0xF118:
    case 0xF218:
    case 0xF318:
    case 0xF418:
    case 0xF518:
    case 0xF618:
    case 0xF718:
    case 0xF818:
    case 0xF918:
    case 0xFA18:
    case 0xFB18:
    case 0xFC18:
    case 0xFD18:
    case 0xFE18:
    case 0xFF18:
        Vx = get_Vx(instruction);
        sound_timer = registers[Vx];
        break;
    case 0xF01E: // Fx1E - ADD I, Vx - Set I = I + Vx.
    case 0xF11E:
    case 0xF21E:
    case 0xF31E:
    case 0xF41E:
    case 0xF51E:
    case 0xF61E:
    case 0xF71E:
    case 0xF81E:
    case 0xF91E:
    case 0xFA1E:
    case 0xFB1E:
    case 0xFC1E:
    case 0xFD1E:
    case 0xFE1E:
    case 0xFF1E:
        index_register += registers[get_Vx(instruction)];
        break;

    case 0xF029: // Fx29 - LD F, Vx - Set I = location of sprite for digit Vx.
    case 0xF129:
    case 0xF229:
    case 0xF329:
    case 0xF429:
    case 0xF529:
    case 0xF629:
    case 0xF729:
    case 0xF829:
    case 0xF929:
    case 0xFA29:
    case 0xFB29:
    case 0xFC29:
    case 0xFD29:
    case 0xFE29:
    case 0xFF29:
        Vx = get_Vx(instruction);
        // FIXME: if problems with rendering, check if 5 is right value here
        index_register = font_start + (Vx * 5);
        break;

    case 0xF033: // Fx33 - LD B, Vx - Store BCD representation of Vx in memory locations I, I+1, and I+2.
    case 0xF133:
    case 0xF233:
    case 0xF333:
    case 0xF433:
    case 0xF533:
    case 0xF633:
    case 0xF733:
    case 0xF833:
    case 0xF933:
    case 0xFA33:
    case 0xFB33:
    case 0xFC33:
    case 0xFD33:
    case 0xFE33:
    case 0xFF33:
        Vx = registers[get_Vx(instruction)];
        memory[index_register] = (Vx / 100) % 10;
        memory[index_register + 1] = (Vx / 10) % 10;
        memory[index_register + 2] = (Vx / 1) % 10;
        break;

    case 0xF055: // Fx55 - LD [I], Vx - Store registers V0 through Vx in memory starting at location I.
    case 0xF155:
    case 0xF255:
    case 0xF355:
    case 0xF455:
    case 0xF555:
    case 0xF655:
    case 0xF755:
    case 0xF855:
    case 0xF955:
    case 0xFA55:
    case 0xFB55:
    case 0xFC55:
    case 0xFD55:
    case 0xFE55:
    case 0xFF55:
        Vx = get_Vx(instruction);
        for (int i = 0; i <= Vx; i++) {
            memory[index_register + i] = registers[i];
        }
        break;

    case 0xF065: // Fx65 - LD Vx, [I] - Read registers V0 through Vx from memory starting at location I.
    case 0xF165:
    case 0xF265:
    case 0xF365:
    case 0xF465:
    case 0xF565:
    case 0xF665:
    case 0xF765:
    case 0xF865:
    case 0xF965:
    case 0xFA65:
    case 0xFB65:
    case 0xFC65:
    case 0xFD65:
    case 0xFE65:
    case 0xFF65:
        Vx = get_Vx(instruction);
        for (int i = 0; i <= Vx; i++) {
            registers[i] = memory[index_register + i];
        }
        break;
    }
}
void Chip8::handle_input()
{
    // TODO: consider this (consider this!) (its a reference to "losing my religion")
    // https://retrocomputing.stackexchange.com/questions/358/how-are-held-down-keys-handled-in-chip-8

    // FIXME: crash when i press CTRL while SDL window is in focus
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

    keypad[chip8_key::ZERO] = KEYS[chip8_sdl_key_translation::KEYPAD_ZERO];
    keypad[chip8_key::ONE] = KEYS[chip8_sdl_key_translation::KEYPAD_ONE];
    keypad[chip8_key::TWO] = KEYS[chip8_sdl_key_translation::KEYPAD_TWO];
    keypad[chip8_key::THREE] = KEYS[chip8_sdl_key_translation::KEYPAD_THREE];
    keypad[chip8_key::FOUR] = KEYS[chip8_sdl_key_translation::KEYPAD_FOUR];
    keypad[chip8_key::FIVE] = KEYS[chip8_sdl_key_translation::KEYPAD_FIVE];
    keypad[chip8_key::SIX] = KEYS[chip8_sdl_key_translation::KEYPAD_SIX];
    keypad[chip8_key::SEVEN] = KEYS[chip8_sdl_key_translation::KEYPAD_SEVEN];
    keypad[chip8_key::EIGHT] = KEYS[chip8_sdl_key_translation::KEYPAD_EIGHT];
    keypad[chip8_key::NINE] = KEYS[chip8_sdl_key_translation::KEYPAD_NINE];
    keypad[chip8_key::LETTER_A] = KEYS[chip8_sdl_key_translation::KEYPAD_A];
    keypad[chip8_key::LETTER_B] = KEYS[chip8_sdl_key_translation::KEYPAD_B];
    keypad[chip8_key::LETTER_C] = KEYS[chip8_sdl_key_translation::KEYPAD_C];
    keypad[chip8_key::LETTER_D] = KEYS[chip8_sdl_key_translation::KEYPAD_D];
    keypad[chip8_key::LETTER_E] = KEYS[chip8_sdl_key_translation::KEYPAD_E];
    keypad[chip8_key::LETTER_F] = KEYS[chip8_sdl_key_translation::KEYPAD_F];
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
