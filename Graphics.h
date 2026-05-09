#pragma once

#include <SDL2/SDL.h>
#include <array>
#include <cstdint>
#include "Chip8.h"

class Graphics {
public:
    Graphics(const char* title, int windowWidth, int windowHeight, int pixelWidth, int pixelHeight);
    ~Graphics();

    auto ReadInput(std::array<uint8_t, NUMBER_OF_KEYS>& keys) -> bool;

    auto Update(const std::array<uint8_t, VIDEO_WIDTH * VIDEO_HEIGHT>& displayBuffer) -> void;

    auto UpdateSound(uint8_t soundTimer) -> void;

private:
    SDL_Window*    m_window{};
    SDL_Renderer*  m_renderer{};
    SDL_AudioDeviceID m_audioDevice{};
    int m_pixelWidth{};
    int m_pixelHeight{};

    double m_sineWavePhase{ 0.0 };
    bool   m_beepPlaying{ false };

    static void AudioCallback(void* userdata, Uint8* stream, int len);
};
