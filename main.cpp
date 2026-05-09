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

    constexpr auto cycleDelay = std::chrono::microseconds(1200);
    bool quit = false;

    while (!quit) {
        quit = window.ReadInput(chip8.m_inputKeys);

        chip8.Cycle();

        window.UpdateSound(chip8.GetSoundTimer());

        if (chip8.m_drawFlag) {
            chip8.m_drawFlag = false;
            window.Update(chip8.m_display);
        }

        std::this_thread::sleep_for(cycleDelay);
    }

    return EXIT_SUCCESS;
}
