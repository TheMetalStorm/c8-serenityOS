#include "Userland/Libraries/LibMain/Main.h"
#include <LibCore/ArgsParser.h>
#include "Chip8.h"

int screen_size_factor = 10;
StringView rom_path;
ErrorOr<int> serenity_main(Main::Arguments arguments){

    //TODO: Timer Timing: when to decrement
    //TODO: CPU Timing - figure out or make input

    Core::ArgsParser parser;
    parser.add_option(screen_size_factor, "The Size of the Emulator Window, 64*screen-size-factor x 32*screen-size-factor. Default is 10.","screen-size-factor", 's', "screen-size-factor");
    parser.add_positional_argument(rom_path, "Path to the Rom", "rom-path");
    parser.parse(arguments);

    auto c8 = Chip8(screen_size_factor);
    TRY(c8.read_rom(rom_path));
    TRY(c8.run());
    return 0;
}
