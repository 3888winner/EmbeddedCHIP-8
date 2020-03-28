#pragma once
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns

char keys[ROWS][COLS] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
    {13, 14, 15, 16},
};

byte rowPins[ROWS] = {5, 6, 7, 8};    //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 10, 11, 12}; //connect to the column pinouts of the keypad


class ChipPad : public Keypad
{
    using Keypad::Keypad;
    
public:
    bool ChipPad::isPressedOrHeld(char keyChar)
    {
        for (byte i = 0; i < LIST_MAX; i++)
        {
            if (key[i].kchar == keyChar)
            {
                if ((key[i].kstate == PRESSED || key[i].kstate == HOLD))
                    return true;
            }
        }
        return false; // Not pressed.
    }
};