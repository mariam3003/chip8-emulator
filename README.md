# CHIP-8 Emulator

A CHIP-8 emulator written in C++ using SDL2 that recreates the functionality of the original CHIP-8 virtual machine. The emulator supports opcode execution, graphics rendering, keyboard input, memory management, timers, and classic CHIP-8 ROM execution.

---

## Overview

CHIP-8 is a simple interpreted programming language originally developed in the 1970s for running games on low-memory systems. This project recreates the CHIP-8 architecture by implementing its CPU, memory, graphics system, timers, stack operations, and input handling from scratch.

The emulator loads and executes CHIP-8 ROMs in real time while reproducing the behaviour of the original virtual machine.

---

## Features

- Full CHIP-8 instruction set implementation
- Opcode fetching, decoding, and execution
- 4KB virtual memory system
- 16-register CPU architecture
- Stack and subroutine handling
- Delay and sound timers
- SDL2 graphics rendering
- Keyboard input mapping
- ROM loading and execution
- Real-time emulation loop

---

## Technologies Used

- C++
- SDL2
- Object-Oriented Programming
- Low-Level Systems Programming

---

## Project Structure

| File | Purpose |
|------|---------|
| `main.cpp` | Main execution loop and emulator startup |
| `Chip8.cpp` | CHIP-8 CPU implementation and opcode execution |
| `Chip8.h` | CPU architecture definitions and declarations |
| `Graphics.cpp` | SDL2 rendering and graphics handling |
| `Graphics.h` | Graphics interface and display management |

---

## Emulator Architecture

The emulator recreates the core CHIP-8 hardware components:

### Memory
- 4KB RAM
- ROMs loaded into program memory starting at `0x200`

### Registers
- 16 general-purpose 8-bit registers (`V0–VF`)
- Index register (`I`)
- Program counter (`PC`)
- Stack pointer (`SP`)

### Timers
- Delay timer
- Sound timer

### Display
- Monochrome 64×32 display buffer
- Sprite rendering through opcode instructions

### Input
- 16-key hexadecimal keypad mapped to keyboard controls

---

## Keyboard Mapping

| CHIP-8 Keypad | Keyboard |
|--------------|----------|
| 1 2 3 C | 1 2 3 4 |
| 4 5 6 D | Q W E R |
| 7 8 9 E | A S D F |
| A 0 B F | Z X C V |

---

## Supported Instructions

The emulator supports:
- Arithmetic operations
- Bitwise operations
- Conditional branching
- Stack calls and returns
- Timers
- Sprite rendering
- Keyboard input instructions
- Memory transfer operations

---

## Requirements

Before running the emulator, install:

- C++17
- SDL2
- g++ or Clang compiler

---

## Installation

### Ubuntu / Debian

```bash
sudo apt install libsdl2-dev
```

### macOS

```bash
brew install sdl2
```

---

## Build & Run

Compile:

```bash
g++ -std=c++17 main.cpp Chip8.cpp Graphics.cpp -lSDL2 -o chip8
```

Run:

```bash
./chip8 roms/tetris.ch8
```

---

## ROM Compatibility

The emulator is designed to run classic CHIP-8 ROMs such as:
- Pong
- Tetris
- Space Invaders
- IBM Logo
- Maze

---

## Challenges & Learning Outcomes

This project provided experience with:
- Emulator architecture
- Opcode decoding
- Memory management
- Real-time graphics rendering
- Event-driven input handling
- Low-level systems programming
- Debugging timing-sensitive applications

---

## Future Improvements

Potential future additions include:
- Super-CHIP support
- Configurable controls
- Save states
- Instruction debugger
- ROM browser UI
- Audio improvements

---

## Notes

This project was developed for educational purposes to explore low-level computer architecture and emulator development concepts.

---

## License

This project is licensed under the MIT License.

---

## Acknowledgments

Inspired by CHIP-8 documentation and emulator development resources, including:

- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
