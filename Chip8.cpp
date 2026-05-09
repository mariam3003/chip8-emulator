#include "Chip8.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>

Chip8::Chip8() {
    m_display.fill(0);
    m_registers.fill(0);
    m_stack.fill(0);
    m_memory.fill(0);
    m_inputKeys.fill(0);

    m_programCounter = START_ADDRESS;
    m_stackPointer   = 0;
    m_index          = 0;
    m_opcode         = 0;
    m_delayTimer     = 0;
    m_soundTimer     = 0;
    m_drawFlag       = false;

    LoadFontSet();
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

auto Chip8::LoadRom(const std::string& filepath) -> bool {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "[Chip8] Failed to open ROM: " << filepath << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    constexpr int maxRomSize = MEMORY_SIZE - START_ADDRESS;
    if (size > maxRomSize) {
        std::cerr << "[Chip8] ROM too large: " << size << " bytes (max " << maxRomSize << ")" << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(&m_memory[START_ADDRESS]), size);
    std::cout << "[Chip8] Loaded ROM: " << filepath << " (" << size << " bytes)" << std::endl;
    return true;
}

auto Chip8::LoadFontSet() -> void {
    for (int i = 0; i < FONT_SIZE; i++) {
        m_memory[FONT_SET_START_ADDRESS + i] = s_fontset[i];
    }
}

auto Chip8::IncrementProgramCounter() -> void {
    m_programCounter += 2;
}

auto Chip8::Cycle() -> void {
    m_opcode = (m_memory[m_programCounter] << 8) | m_memory[m_programCounter + 1];
    IncrementProgramCounter();

    const uint8_t  vx  = (m_opcode & 0x0F00) >> 8;
    const uint8_t  vy  = (m_opcode & 0x00F0) >> 4;
    const uint8_t  kk  =  m_opcode & 0x00FF;
    const uint16_t nnn =  m_opcode & 0x0FFF;
    const uint8_t  n   =  m_opcode & 0x000F;

    switch (m_opcode & 0xF000) {

        case 0x0000:
            switch (kk) {
                case 0xE0:
                    m_display.fill(0);
                    m_drawFlag = true;
                    break;

                case 0xEE:
                    --m_stackPointer;
                    m_programCounter = m_stack[m_stackPointer];
                    break;

                default:
                    std::cerr << "[Chip8] Unknown opcode: " << std::hex << m_opcode << std::endl;
                    break;
            }
            break;

        case 0x1000:
            m_programCounter = nnn;
            break;

        case 0x2000:
            m_stack[m_stackPointer] = m_programCounter;
            ++m_stackPointer;
            m_programCounter = nnn;
            break;

        case 0x3000:
            if (m_registers[vx] == kk)
                IncrementProgramCounter();
            break;

        case 0x4000:
            if (m_registers[vx] != kk)
                IncrementProgramCounter();
            break;

        case 0x5000:
            if (m_registers[vx] == m_registers[vy])
                IncrementProgramCounter();
            break;

        case 0x6000:
            m_registers[vx] = kk;
            break;

        case 0x7000:
            m_registers[vx] += kk;
            break;

        case 0x8000:
            switch (n) {
                case 0x0:
                    m_registers[vx] = m_registers[vy];
                    break;

                case 0x1:
                    m_registers[vx] |= m_registers[vy];
                    break;

                case 0x2:
                    m_registers[vx] &= m_registers[vy];
                    break;

                case 0x3:
                    m_registers[vx] ^= m_registers[vy];
                    break;

                case 0x4: {
                    uint16_t sum     = m_registers[vx] + m_registers[vy];
                    m_registers[0xF] = (sum > 0xFF) ? 1 : 0;
                    m_registers[vx]  = sum & 0xFF;
                    break;
                }

                case 0x5:
                    m_registers[0xF] = (m_registers[vx] > m_registers[vy]) ? 1 : 0;
                    m_registers[vx] -= m_registers[vy];
                    break;

                case 0x6:
                    m_registers[0xF] = m_registers[vx] & 0x1;
                    m_registers[vx] >>= 1;
                    break;

                case 0x7:
                    m_registers[0xF] = (m_registers[vy] > m_registers[vx]) ? 1 : 0;
                    m_registers[vx]  = m_registers[vy] - m_registers[vx];
                    break;

                case 0xE:
                    m_registers[0xF] = (m_registers[vx] >> 7) & 0x1;
                    m_registers[vx] <<= 1;
                    break;

                default:
                    std::cerr << "[Chip8] Unknown opcode: " << std::hex << m_opcode << std::endl;
                    break;
            }
            break;

        case 0x9000:
            if (m_registers[vx] != m_registers[vy])
                IncrementProgramCounter();
            break;

        case 0xA000:
            m_index = nnn;
            break;

        case 0xB000:
            m_programCounter = nnn + m_registers[0];
            break;

        case 0xC000:
            m_registers[vx] = static_cast<uint8_t>(std::rand() % 256) & kk;
            break;

        case 0xD000: {
            uint8_t xPos = m_registers[vx] % VIDEO_WIDTH;
            uint8_t yPos = m_registers[vy] % VIDEO_HEIGHT;
            m_registers[0xF] = 0;

            for (int row = 0; row < n; ++row) {
                uint8_t spriteByte = m_memory[m_index + row];
                for (int col = 0; col < 8; ++col) {
                    if (spriteByte & (0x80 >> col)) {
                        int idx = ((yPos + row) % VIDEO_HEIGHT) * VIDEO_WIDTH
                                + ((xPos + col) % VIDEO_WIDTH);
                        if (m_display[idx] == 1)
                            m_registers[0xF] = 1;
                        m_display[idx] ^= 1;
                    }
                }
            }
            m_drawFlag = true;
            break;
        }

        case 0xE000:
            switch (kk) {
                case 0x9E:
                    if (m_inputKeys[m_registers[vx]])
                        IncrementProgramCounter();
                    break;

                case 0xA1:
                    if (!m_inputKeys[m_registers[vx]])
                        IncrementProgramCounter();
                    break;

                default:
                    std::cerr << "[Chip8] Unknown opcode: " << std::hex << m_opcode << std::endl;
                    break;
            }
            break;

        case 0xF000:
            switch (kk) {
                case 0x07:
                    m_registers[vx] = m_delayTimer;
                    break;

                case 0x0A: {
                    bool keyFound = false;
                    for (int i = 0; i < NUMBER_OF_KEYS; ++i) {
                        if (m_inputKeys[i]) {
                            m_registers[vx] = static_cast<uint8_t>(i);
                            keyFound = true;
                            break;
                        }
                    }
                    if (!keyFound)
                        m_programCounter -= 2;
                    break;
                }

                case 0x15:
                    m_delayTimer = m_registers[vx];
                    break;

                case 0x18:
                    m_soundTimer = m_registers[vx];
                    break;

                case 0x1E:
                    m_index += m_registers[vx];
                    break;

                case 0x29:
                    m_index = FONT_SET_START_ADDRESS + (m_registers[vx] * 5);
                    break;

                case 0x33:
                    m_memory[m_index]     = m_registers[vx] / 100;
                    m_memory[m_index + 1] = (m_registers[vx] / 10) % 10;
                    m_memory[m_index + 2] = m_registers[vx] % 10;
                    break;

                case 0x55:
                    for (int i = 0; i <= vx; ++i)
                        m_memory[m_index + i] = m_registers[i];
                    break;

                case 0x65:
                    for (int i = 0; i <= vx; ++i)
                        m_registers[i] = m_memory[m_index + i];
                    break;

                default:
                    std::cerr << "[Chip8] Unknown opcode: " << std::hex << m_opcode << std::endl;
                    break;
            }
            break;

        default:
            std::cerr << "[Chip8] Unknown opcode: " << std::hex << m_opcode << std::endl;
            break;
    }

    if (m_delayTimer > 0)
        --m_delayTimer;

    if (m_soundTimer > 0)
        --m_soundTimer;
}
