#include "Userland/Libraries/LibMain/Main.h"
#include "AK/Format.h"
#include "Chip8.h"
#include "stack.h"

ErrorOr<int> serenity_main(Main::Arguments){

    Chip8 c8 = Chip8();

    TRY(c8.read_rom("/home/anon/Chip8Picture.ch8"sv));
    c8.run();
    return 0;
}
