#pragma once
#include <Arduino.h>
#include <String.h>
#include <avr/pgmspace.h>

/*
0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
0x200-0xFFF - Program ROM and work RAM
*/

class CPU{
    public:
        CPU();
        ~CPU();

    private:
        // Registers & Timers
        unsigned short I;
        unsigned short sp;
        unsigned short pc;
        unsigned short opcode;
        unsigned char delay_timer;
        unsigned char sound_timer;
        unsigned char V[16];
        unsigned char memory[4096];
        unsigned short stack[16];

    public:
        unsigned char gfx[64][32];
        unsigned char key[16];
        bool drawFlag;
        bool soundFlag;

    public:
        void InitializeMemory();
        void LoadGame(const unsigned char game[], size_t size);
        void EmulateCycle();
        void DebugGraphics();
    
    private:
        template<class OutputIterator, class Size, class T>
        void fill_n(OutputIterator __first, Size size, const T& val);
        
    private:
        unsigned char chip8_fontset[80] =
            {
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };
};