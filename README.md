# CHIP-8 Emulator for SerenityOS

A CHIP-8 emulator built specifically for the SerenityOS operating system, featuring SDL2 graphics, audio support, and configurable CPU timing.

## Features

- **Complete CHIP-8 instruction set** - Supports all standard CHIP-8 opcodes
- **SDL2 graphics** - Hardware-accelerated rendering with configurable window scaling
- **Audio support** - 440Hz square wave beep for sound timer
- **Configurable CPU timing** - Adjustable execution speed from slow to very fast
- **Proper input handling** - Full 16-key CHIP-8 keypad mapping
- **SerenityOS integration** - Uses SerenityOS ErrorOr pattern and memory management

## Building

This project is designed for SerenityOS. Follow these steps to build and install:

### Prerequisites
First, follow the [SerenityOS build instructions](https://github.com/SerenityOS/serenity/blob/master/Documentation/BuildInstructions.md) to set up your development environment.

### Installation
1. **Place the source code** in the SerenityOS applications directory:
   ```
   serenity/Userland/Applications/c8-serenityOS/
   ├── rom/
   ├── src/
   ├── CMakeLists.txt
   └── README.md
   ```

2. **Add to CMake configuration** by editing `serenity/Userland/Applications/CMakeLists.txt`:
   ```cmake
   add_subdirectory(c8-serenityOS)
   ```

3. **Build SerenityOS** following the standard build process:
   ```bash
   serenity/Meta/serenity.sh run
   ```
   The emulator will be available in the system after boot.

## Usage

Once installed in SerenityOS, run the emulator from the terminal:

```bash
c8 [OPTIONS] <rom-path>
```

### Options

- `-s, --screen-size-factor <factor>` - Window scaling factor (default: 10)
  - Creates a window of size `64×factor x 32×factor` pixels
- `-c, --cpu-timing <hz>` - CPU speed in cycles per second (default: 700)
  - Standard CHIP-8 timing is ~700Hz
  - Lower values = slower execution, higher values = faster execution

### Examples

```bash
# Run with default settings
c8 rom/IBM\ Logo.ch8

# Run with larger window and slower timing
c8 -s 15 -c 500 rom/test/1-chip8-logo.ch8

# Run with faster timing for games
c8 -c 2000 rom/test/3-corax+.ch8
```

## Controls

The CHIP-8 keypad is mapped to QWERTY keyboard as follows:

```
CHIP-8 Keypad    QWERTY Keyboard
┌─┬─┬─┬─┐       ┌─┬─┬─┬─┐
│1│2│3│C│  -->  │1│2│3│4│
├─┼─┼─┼─┤       ├─┼─┼─┼─┤
│4│5│6│D│  -->  │Q│W│E│R│
├─┼─┼─┼─┤       ├─┼─┼─┼─┤
│7│8│9│E│  -->  │A│S│D│F│
├─┼─┼─┼─┤       ├─┼─┼─┼─┤
│A│0│B│F│  -->  │Y│X│C│V│
└─┴─┴─┴─┘       └─┴─┴─┴─┘
```

- **ESC** - Exit emulator
- **Ctrl+C** - Force quit (terminal)

## ROMs

The `rom/` directory contains test ROMs for verification:

- `IBM Logo.ch8` - Classic IBM logo demo
- `Chip8Picture.ch8` - Graphics test
- `rom/test/` - Test suite including:
  - `1-chip8-logo.ch8` - Logo test
  - `2-ibm-logo.ch8` - IBM logo
  - `3-corax+.ch8` - Extended instruction test
  - `4-flags.ch8` - Flag register tests
  - `5-quirks.ch8` - Compatibility tests
  - `6-keypad.ch8` - Input testing
  - `7-beep.ch8` - Audio test
  - `8-scrolling.ch8` - Display tests

## Technical Details

### Architecture

- **CPU Timing**: Configurable instruction execution rate with 60Hz timer system
- **Graphics**: 64×32 monochrome display with sprite-based drawing
- **Audio**: SDL audio with 440Hz square wave generation
- **Memory**: 4KB RAM with program loading at 0x200
- **Input**: 16-key hexadecimal keypad

### Implementation Notes

- Uses SerenityOS `ErrorOr<T>` pattern for proper error handling
- SDL resources managed with RAII and proper cleanup ordering
- Timer system runs independently at 60Hz regardless of CPU timing
- Sprite drawing includes proper bounds checking and collision detection
- Audio system uses low-latency buffering for responsive sound

## Known Issues

- **Window close crash**: Closing the emulator window with the X button currently causes a crash. Use ESC key or Ctrl+C to exit cleanly instead.

## Development

The codebase follows SerenityOS conventions:

- `src/Chip8.cpp` - Main emulator logic and instruction decoding
- `src/screen.cpp` - SDL graphics and audio management  
- `src/main.cpp` - Application entry point and argument parsing
- `src/stack.h` - Call stack implementation

### Contributing

When contributing, please:
- Follow SerenityOS coding style
- Use `ErrorOr<T>` for error handling
- Include proper bounds checking
- Test with the provided ROM suite

## License

This project follows SerenityOS licensing.