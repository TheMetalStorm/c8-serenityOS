#include "Userland/Libraries/LibMain/Main.h"
#include "Chip8.h"

ErrorOr<int> serenity_main(Main::Arguments){

    auto c8 = Chip8();
    TRY(c8.read_rom("/home/anon/Chip8Picture.ch8"sv));
    TRY(c8.run());
    return 0;
}
