#include "CPU.hpp"
#include <avr/pgmspace.h>
#include "ChipPad.h"
#include <Keypad.h>

ChipPad keypad = ChipPad(((char*)keys), rowPins, colPins, ROWS, COLS);

CPU::CPU() {}
CPU::~CPU() {}

template<class OutputIterator, class Size, class T>
void CPU::fill_n(OutputIterator __first, Size n, const T& val){

    while(n > 0){
        *__first = val;
        ++__first;--n;
    }

}

void CPU::InitializeMemory()
{
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;
    drawFlag = false;
    soundFlag = false;

    fill_n(V, 16, 0);
    fill_n(memory, 4096, 0);
    
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 32; j++){
            gfx[i][j] = 0;
        }
    }

    fill_n(stack, 16, 0);
    fill_n(key, 16, 0);

    
    for (int i = 0; i < 80; i++)
        memory[i] = chip8_fontset[i];

    delay_timer = 0;
    sound_timer = 0;

}


void CPU::LoadGame(const unsigned char game[], size_t size)
{

        if (size + 512 < 4096)
        {
            for (int i = 512; i < 512 + size; i++){
                memory[i] = pgm_read_byte(game +(i-512));
                Serial.println(game[i-512]);
                }
        }

        Serial.println("Done");
}


void CPU::EmulateCycle()
{
    // Fetch
    opcode = (memory[pc] << 8) | (memory[pc + 1]);
    Serial.println(opcode,HEX);
    // Decode & Cycle
    switch (opcode & 0xF000)
    {
    case 0:
        switch (opcode & 0x000F)
        {
        case 0:
            // CLS (0x0000) - Clear Screen
            for(int i = 0; i < 64; i++){
                for(int j = 0; j < 32; j++){
                    gfx[i][j] = 0;
                }
            }

            drawFlag = true;
            pc += 2;
            break;

        case 0x000E:
            // RTS (0x000E) - Return from subroutine
            --sp;
            pc = stack[sp];
            pc+=2;
            break;
        }

        break;

    case 0x1000:
        // JP (0x1nnn) - Jump to address nnn
        pc = opcode & 0x0FFF;
        break;

    case 0x2000:
        // CALL (0x2nnn) - Call subroutine at nnn
        stack[sp] = pc;
        ++sp;
        pc = opcode & 0x0FFF;
        break;

    case 0x3000:
        // SE (0x3xkk) - Skip next instr if Vx == kk
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
        {
            pc += 4;
        }
        else
        {
            pc += 2;
        }
        break;

    case 0x4000:
        // SNE (0x4xkk) - Skip next instr if Vx != kk
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
        {
            pc += 4;
        }
        else
        {
            pc += 2;
        }
        break;

    case 0x5000:
        // SE (0x5xy0) - Skip next instruction if Vx = Vy
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
        {
            pc += 4;
        }
        else
        {
            pc += 2;
        }
        break;

    case 0x6000:
        // LD (0x6xkk) - Set Vx = kk
        V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
        pc += 2;
        break;

    case 0x7000:
        // ADD (0x7xkk) - Set Vx = Vx + kk
        V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
        pc += 2;
        break;

    case 0x8000:
        switch (opcode & 0x000F)
        {
        case 0:
            // LD (0x8xy0) - Stores the value of register Vy in register Vx
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0001:
            // OR (0x8xy1) - Set Vx = Vx OR Vy
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0002:
            // AND (0x8xy2) - Set Vx = Vx AND Vy
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0003:
            // XOR (0x8xy3) - Set Vx = Vx XOR Vy
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0004:
            // ADD (0x8xy4)- Set Vx = Vx + Vy, set VF = carry

            if (V[(opcode & 0x0F00) >> 8] > (0xFF - V[(opcode & 0x00F0) >> 4]))
            {
                V[15] = 1;
            }
            else
            {
                V[15] = 0;
            }
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0005:
            // SUB (0x8xy5) - Set Vx = Vx - Vy, set VF = NOT borrow
            if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
            {
                V[15] = 1;
            }
            else
            {
                V[15] = 0;
            }

            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0006:
            // SHR (0x8xyE) - Set Vx = Vx SHR 1
            V[15] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            pc += 2;
            break;

        case 0x0007:
            // SUBN (0x8xy7) - Set Vx = Vy - Vx, set VF = NOT borrow
            if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4])
            {
                V[15] = 1;
            }
            else
            {
                V[15] = 0;
            }

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x000E:
            // SHL (0x8xyE) - Set Vx = Vx SHL 1
            V[15] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            pc += 2;
            break;
        }
        break;

    case 0x9000:
        // SNE (0x9xy0) - Skip next instruction if Vx != Vy
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
        {
            pc += 4;
        }
        else
        {
            pc += 2;
        }

    case 0xA000:
        // LD (0xAnnn) -  Set I = nnn
        I = (opcode & 0x0FFF);
        pc += 2;
        break;

    case 0xB000:
        // JP (0xBnnn) - Jump to location nnn + V0
        pc = (opcode & 0x0FFF) + V[0];
        break;

    case 0xC000:
        //RND (0xCxkk) - Set Vx = random byte AND kk.
        V[(opcode & 0x0F00) >> 8] = (random(0,256)) & (opcode & 0x00FF);
        pc += 2;
        break;

    case 0xD000:
    {
        // DRW (0xDxyn) - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        unsigned short x = V[(opcode & 0x0F00) >> 8];
        unsigned short y = V[(opcode & 0x00F0) >> 4];
        unsigned short height = (opcode & 0x000F);
        unsigned short pixel;

        V[15] = 0;

        for (int ycor = 0; ycor < height; ycor++)
        {

            pixel = memory[I + ycor];

            for (int xcor = 0; xcor < 8; xcor++)
            {
                if ((pixel & (0x80 >> xcor)) != 0)
                {
                    if (gfx[x + xcor][(y + ycor)] == 1)
                    {
                        V[15] = 1;
                    }
                    gfx[x + xcor][y + ycor] ^= 1;
                }
            }
        }
        drawFlag = true;
        pc += 2;
    }
    break;

    case 0xE000:
        switch (opcode & 0x00F0)
        {
        case 0x0090:
            // SKP (0xEx9E) - Skip next instruction if key with the value of Vx is pressed
            keypad.getKeys();
            if (keypad.isPressedOrHeld(V[(opcode & 0x0F00) >> 8]))
            {
                pc += 4;

            }
            else
            {
                pc += 2;
            }
            break;

        case 0x00A0:
            // SKNP (0xExA1) - Skip next instruction if key with the value of Vx is not pressed
            keypad.getKeys();
            if (!keypad.isPressedOrHeld(V[(opcode & 0x0F00) >> 8]))
            {
                pc += 4;
                
            }
            else
            {
                pc += 2;
            }
            break;
        }
        break;

    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007:
            //LD (0xFx07) - Set Vx = delay timer value
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            pc += 2;
            break;

        case 0x000A:
        {
            //LD (0xFx0A) - Wait for a key press, store the value of the key in Vx
            bool keyPress = false;

            for (int i = 0; i < 16; ++i)
            {
                if (key[i] != 0)
                {
                    V[(opcode & 0x0F00) >> 8] = i;
                    keyPress = true;
                }
            }

            if (!keyPress)
                return;
            pc += 2;
        }
        break;

        case 0x0015:
            // LD (0xFx15) - Set delay timer = Vx
            delay_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x0018:
            // LD (0xFx18) - Set sound timer = Vx
            sound_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x001E:
            // ADD (0xFx1E) - Set I = I + Vx.
            if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                V[0xF] = 1;
            else
                V[0xF] = 0;

            I += V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x0029:
            // LD (0xFx29) - Set I = location of sprite for digit Vx
            I = V[(opcode & 0x0F00) >> 8] * 0x5;
            pc += 2;
            break;

        case 0x0033:
            // LD (0xFx33) - Store BCD representation of Vx in memory locations I, I+1, and I+2
            memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
            pc += 2;
            break;

        case 0x0055: 
            // LD (0xFx55) - Stores V0 to VX in memory starting at address I					
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                memory[I + i] = V[i];	

            pc += 2;
        break;

        case 0x0065: 
            // LD (0xFx65) - Fills V0 to VX with values from memory starting at address I					
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                V[i] = memory[I + i];			

            pc += 2;
        break;
        }

        break;
    }

    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0)
    {
        if (sound_timer == 1)
            soundFlag = true;
    
        --sound_timer;
    }

}


void CPU::DebugGraphics()
{
	// Draw
	for(int y = 0; y < 32; ++y)
	{
		for(int x = 0; x < 64; ++x)
		{
			if(gfx[x][y] == 0) 
				Serial.print("O");
			else 
				Serial.print(" ");
		}
		Serial.println();
	}
	Serial.println();

}