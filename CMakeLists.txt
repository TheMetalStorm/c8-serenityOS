serenity_component(
    c8
    RECOMMENDED
    TARGETS c8
)

set(SOURCES
        src/main.cpp src/Chip8.cpp src/screen.cpp
)

serenity_app(c8 ICON app-calculator)
find_package(SDL2 QUIET)
target_include_directories(c8 PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(c8 PRIVATE LibMain LibCore LibGfx LibMedia SDL2::SDL2)
