#include "Chip8.h"
#include "Graphics.h"

#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path/to/rom>" << std::endl;
        return EXIT_FAILURE;
    }

    constexpr int windowWidth  = 1024;
    constexpr int windowHeight = 512;
    constexpr int pixelWidth   = windowWidth  / VIDEO_WIDTH;
    constexpr int pixelHeight  = windowHeight / VIDEO_HEIGHT;

    Chip8    chip8;
    Graphics window("CHIP-8 Emulator", windowWidth, windowHeight, pixelWidth, pixelHeight);

    if (!chip8.LoadRom(argv[1]))
        return EXIT_FAILURE;

    // ~1200 microseconds per cycle gives roughly 500-600 cycles/sec
    constexpr auto cycleDelay = std::chrono::microseconds(1200);
    bool quit = false;

    while (!quit) {
        quit = window.ReadInput(chip8.m_inputKeys);

        chip8.Cycle();

        // Play beep if sound timer is active
        window.UpdateSound(chip8.GetSoundTimer());

        // Only redraw when the display has actually changed
        if (chip8.m_drawFlag) {
            chip8.m_drawFlag = false;
            window.Update(chip8.m_display);
        }

        std::this_thread::sleep_for(cycleDelay);
    }

    return EXIT_SUCCESS;
}
