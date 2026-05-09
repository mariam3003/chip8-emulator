#include "Graphics.h"

#include <iostream>
#include <unordered_map>
#include <cmath>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

void Graphics::AudioCallback(void* userdata, Uint8* stream, int len) {
    auto* self = static_cast<Graphics*>(userdata);
    Sint16* buffer = reinterpret_cast<Sint16*>(stream);
    int samples = len / 2;

    constexpr double frequency  = 440.0;
    constexpr double sampleRate = 44100.0;
    constexpr double amplitude  = 8000.0;

    for (int i = 0; i < samples; ++i) {
        buffer[i] = static_cast<Sint16>(amplitude * std::sin(self->m_sineWavePhase));
        self->m_sineWavePhase += 2.0 * M_PI * frequency / sampleRate;
        if (self->m_sineWavePhase > 2.0 * M_PI)
            self->m_sineWavePhase -= 2.0 * M_PI;
    }
}

Graphics::Graphics(const char* title, int windowWidth, int windowHeight, int pixelWidth, int pixelHeight)
    : m_pixelWidth(pixelWidth), m_pixelHeight(pixelHeight)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "[Graphics] Failed to initialize SDL: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_SHOWN
    );
    if (!m_window) {
        std::cerr << "[Graphics] Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        std::cerr << "[Graphics] Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }

    SDL_AudioSpec audioSpec{};
    audioSpec.freq     = 44100;
    audioSpec.format   = AUDIO_S16SYS;
    audioSpec.channels = 1;
    audioSpec.samples  = 512;
    audioSpec.callback = AudioCallback;
    audioSpec.userdata = this;

    m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, 0);
    if (m_audioDevice == 0) {
        std::cerr << "[Graphics] Failed to open audio device: " << SDL_GetError() << std::endl;
    }
}

Graphics::~Graphics() {
    if (m_audioDevice) SDL_CloseAudioDevice(m_audioDevice);
    if (m_renderer)    SDL_DestroyRenderer(m_renderer);
    if (m_window)      SDL_DestroyWindow(m_window);
    SDL_Quit();
}

auto Graphics::Update(const std::array<uint8_t, VIDEO_WIDTH * VIDEO_HEIGHT>& displayBuffer) -> void {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    for (int y = 0; y < VIDEO_HEIGHT; ++y) {
        for (int x = 0; x < VIDEO_WIDTH; ++x) {
            if (displayBuffer[y * VIDEO_WIDTH + x]) {
                SDL_Rect rect = {
                    x * m_pixelWidth,
                    y * m_pixelHeight,
                    m_pixelWidth,
                    m_pixelHeight
                };
                SDL_RenderFillRect(m_renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(m_renderer);
}

auto Graphics::UpdateSound(uint8_t soundTimer) -> void {
    if (m_audioDevice == 0) return;

    if (soundTimer > 0 && !m_beepPlaying) {
        SDL_PauseAudioDevice(m_audioDevice, 0);
        m_beepPlaying = true;
    } else if (soundTimer == 0 && m_beepPlaying) {
        SDL_PauseAudioDevice(m_audioDevice, 1);
        m_beepPlaying = false;
    }
}

auto Graphics::ReadInput(std::array<uint8_t, NUMBER_OF_KEYS>& keys) -> bool {
    static const std::unordered_map<SDL_Keycode, uint8_t> keyMapping = {
        {SDLK_x, 0x0}, {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3},
        {SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_a, 0x7},
        {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_z, 0xA}, {SDLK_c, 0xB},
        {SDLK_4, 0xC}, {SDLK_r, 0xD}, {SDLK_f, 0xE}, {SDLK_v, 0xF}
    };

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return true;

            case SDL_KEYDOWN: {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    return true;
                auto it = keyMapping.find(event.key.keysym.sym);
                if (it != keyMapping.end())
                    keys[it->second] = 1;
                break;
            }

            case SDL_KEYUP: {
                auto it = keyMapping.find(event.key.keysym.sym);
                if (it != keyMapping.end())
                    keys[it->second] = 0;
                break;
            }
        }
    }
    return false;
}
