#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "CPU.hpp"


//#include "Tetris.h"
#include "Blitz.h"


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define OLED_RESET -1

CPU cpu = CPU();
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void draw();

void setup()
{

  Serial.begin(9600);
  randomSeed(analogRead(0));

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();

  cpu.InitializeMemory();
  cpu.LoadGame(BRIX, sizeof(BRIX));
}

void loop()
{

  cpu.EmulateCycle();

  if (cpu.drawFlag)
  {
    draw();
    cpu.drawFlag = false;
  }

  delay(17);
}

void draw()
{
  for (int x = 0; x < 64; x++)
  {
    for (int y = 0; y < 32; y++)
    {
      if (cpu.gfx[x][y] == 1)
      {
        display.drawRect(x * 2, y * 2, 2, 2, SSD1306_WHITE);
      }
      else
      {
        display.drawRect(x * 2, y * 2, 2, 2, SSD1306_BLACK);
      }
    }
  }
  display.display();
}