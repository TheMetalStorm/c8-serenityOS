serenity_component(
    c8
    RECOMMENDED
    TARGETS c8
)

set(SOURCES
        src/main.cpp src/Chip8.cpp
)

serenity_app(c8 ICON app-calculator)
target_link_libraries(c8 PRIVATE LibCore LibMain)
