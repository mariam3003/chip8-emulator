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

    // Seed RNG once at startup
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

    // Make sure the ROM fits in available memory
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
    // --- Fetch ---
    m_opcode = (m_memory[m_programCounter] << 8) | m_memory[m_programCounter + 1];
    IncrementProgramCounter();

    // Decode common nibbles upfront to keep cases clean
    const uint8_t  vx  = (m_opcode & 0x0F00) >> 8; // register X index
    const uint8_t  vy  = (m_opcode & 0x00F0) >> 4; // register Y index
    const uint8_t  kk  =  m_opcode & 0x00FF;        // lower byte
    const uint16_t nnn =  m_opcode & 0x0FFF;        // lower 12 bits
    const uint8_t  n   =  m_opcode & 0x000F;        // lower nibble

    // --- Decode & Execute ---
    switch (m_opcode & 0xF000) {

        // 0x00E_ - System opcodes
        case 0x0000:
            switch (kk) {
                // 00E0 - Clear display
                case 0xE0:
                    m_display.fill(0);
                    m_drawFlag = true;
                    break;

                // 00EE - Return from subroutine
                case 0xEE:
                    --m_stackPointer;
                    m_programCounter = m_stack[m_stackPointer];
                    break;

                default:
                    std::cerr << "[Chip8] Unknown opcode: " << std::hex << m_opcode << std::endl;
                    break;
            }
            break;

        // 1NNN - Jump to address NNN
        case 0x1000:
            m_programCounter = nnn;
            break;

        // 2NNN - Call subroutine at NNN
        case 0x2000:
            m_stack[m_stackPointer] = m_programCounter;
            ++m_stackPointer;
            m_programCounter = nnn;
            break;

        // 3XKK - Skip next instruction if Vx == kk
        case 0x3000:
            if (m_registers[vx] == kk)
                IncrementProgramCounter();
            break;

        // 4XKK - Skip next instruction if Vx != kk
        case 0x4000:
            if (m_registers[vx] != kk)
                IncrementProgramCounter();
            break;

        // 5XY0 - Skip next instruction if Vx == Vy
        case 0x5000:
            if (m_registers[vx] == m_registers[vy])
                IncrementProgramCounter();
            break;

        // 6XKK - Set Vx = kk
        case 0x6000:
            m_registers[vx] = kk;
            break;

        // 7XKK - Set Vx = Vx + kk
        case 0x7000:
            m_registers[vx] += kk;
            break;

        // 8XY_ - Arithmetic and bitwise operations
        case 0x8000:
            switch (n) {
                // 8XY0 - Set Vx = Vy
                case 0x0:
                    m_registers[vx] = m_registers[vy];
                    break;

                // 8XY1 - Set Vx = Vx OR Vy
                case 0x1:
                    m_registers[vx] |= m_registers[vy];
                    break;

                // 8XY2 - Set Vx = Vx AND Vy
                case 0x2:
                    m_registers[vx] &= m_registers[vy];
                    break;

                // 8XY3 - Set Vx = Vx XOR Vy
                case 0x3:
                    m_registers[vx] ^= m_registers[vy];
                    break;

                // 8XY4 - Set Vx = Vx + Vy, VF = carry
                case 0x4: {
                    uint16_t sum     = m_registers[vx] + m_registers[vy];
                    m_registers[0xF] = (sum > 0xFF) ? 1 : 0;
                    m_registers[vx]  = sum & 0xFF;
                    break;
                }

                // 8XY5 - Set Vx = Vx - Vy, VF = NOT borrow (1 if no borrow)
                case 0x5:
                    m_registers[0xF] = (m_registers[vx] > m_registers[vy]) ? 1 : 0;
                    m_registers[vx] -= m_registers[vy];
                    break;

                // 8XY6 - Shift Vx right by 1, VF = LSB before shift
                case 0x6:
                    m_registers[0xF] = m_registers[vx] & 0x1;
                    m_registers[vx] >>= 1;
                    break;

                // 8XY7 - Set Vx = Vy - Vx, VF = NOT borrow (1 if no borrow)
                case 0x7:
                    m_registers[0xF] = (m_registers[vy] > m_registers[vx]) ? 1 : 0;
                    m_registers[vx]  = m_registers[vy] - m_registers[vx];
                    break;

                // 8XYE - Shift Vx left by 1, VF = MSB before shift
                case 0xE:
                    m_registers[0xF] = (m_registers[vx] >> 7) & 0x1;
                    m_registers[vx] <<= 1;
                    break;

                default:
                    std::cerr << "[Chip8] Unknown opcode: " << std::hex << m_opcode << std::endl;
                    break;
            }
            break; // end 0x8000

        // 9XY0 - Skip next instruction if Vx != Vy
        case 0x9000:
            if (m_registers[vx] != m_registers[vy])
                IncrementProgramCounter();
            break;

        // ANNN - Set I = NNN
        case 0xA000:
            m_index = nnn;
            break;

        // BNNN - Jump to address NNN + V0
        case 0xB000:
            m_programCounter = nnn + m_registers[0];
            break;

        // CXKK - Set Vx = random byte AND kk
        case 0xC000:
            m_registers[vx] = static_cast<uint8_t>(std::rand() % 256) & kk;
            break;

        // DXYN - Draw N-byte sprite at (Vx, Vy), VF = collision flag
        case 0xD000: {
            uint8_t xPos = m_registers[vx] % VIDEO_WIDTH;
            uint8_t yPos = m_registers[vy] % VIDEO_HEIGHT;
            m_registers[0xF] = 0;

            for (int row = 0; row < n; ++row) {
                uint8_t spriteByte = m_memory[m_index + row];
                for (int col = 0; col < 8; ++col) {
                    if (spriteByte & (0x80 >> col)) {
                        // Wrap sprite around screen edges
                        int idx = ((yPos + row) % VIDEO_HEIGHT) * VIDEO_WIDTH
                                + ((xPos + col) % VIDEO_WIDTH);
                        // Set collision flag if a lit pixel gets turned off
                        if (m_display[idx] == 1)
                            m_registers[0xF] = 1;
                        m_display[idx] ^= 1;
                    }
                }
            }
            m_drawFlag = true;
            break;
        }

        // EX__ - Key input operations
        case 0xE000:
            switch (kk) {
                // EX9E - Skip next instruction if key Vx is pressed
                case 0x9E:
                    if (m_inputKeys[m_registers[vx]])
                        IncrementProgramCounter();
                    break;

                // EXA1 - Skip next instruction if key Vx is NOT pressed
                case 0xA1:
                    if (!m_inputKeys[m_registers[vx]])
                        IncrementProgramCounter();
                    break;

                default:
                    std::cerr << "[Chip8] Unknown opcode: " << std::hex << m_opcode << std::endl;
                    break;
            }
            break; // end 0xE000

        // FX__ - Miscellaneous operations
        case 0xF000:
            switch (kk) {
                // FX07 - Set Vx = delay timer value
                case 0x07:
                    m_registers[vx] = m_delayTimer;
                    break;

                // FX0A - Wait for a key press, store key in Vx
                // Non-blocking: if no key is pressed, rewind PC and retry next cycle
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

                // FX15 - Set delay timer = Vx
                case 0x15:
                    m_delayTimer = m_registers[vx];
                    break;

                // FX18 - Set sound timer = Vx
                case 0x18:
                    m_soundTimer = m_registers[vx];
                    break;

                // FX1E - Set I = I + Vx
                case 0x1E:
                    m_index += m_registers[vx];
                    break;

                // FX29 - Set I = location of font sprite for digit Vx
                case 0x29:
                    m_index = FONT_SET_START_ADDRESS + (m_registers[vx] * 5);
                    break;

                // FX33 - Store BCD representation of Vx at I, I+1, I+2
                case 0x33:
                    m_memory[m_index]     = m_registers[vx] / 100;        // hundreds
                    m_memory[m_index + 1] = (m_registers[vx] / 10) % 10;  // tens
                    m_memory[m_index + 2] = m_registers[vx] % 10;         // ones
                    break;

                // FX55 - Store registers V0 through Vx in memory starting at I
                case 0x55:
                    for (int i = 0; i <= vx; ++i)
                        m_memory[m_index + i] = m_registers[i];
                    break;

                // FX65 - Read registers V0 through Vx from memory starting at I
                case 0x65:
                    for (int i = 0; i <= vx; ++i)
                        m_registers[i] = m_memory[m_index + i];
                    break;

                default:
                    std::cerr << "[Chip8] Unknown opcode: " << std::hex << m_opcode << std::endl;
                    break;
            }
            break; // end 0xF000

        default:
            std::cerr << "[Chip8] Unknown opcode: " << std::hex << m_opcode << std::endl;
            break;
    }

    // --- Update timers ---
    if (m_delayTimer > 0)
        --m_delayTimer;

    if (m_soundTimer > 0)
        --m_soundTimer;
}
