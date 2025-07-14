/**
 *  Copyright 2025 Niklas Meyer
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <AnimatedGIF.h>

#include "Fonts/FreeMonoBold18pt7b.h"

#define PANEL_RES_X 64  // vertical number of pixels per display
#define PANEL_RES_Y 32  // vertical number of pixels per display
#define PANEL_CHAIN 2   // number of displays

#define R1_PIN 36
#define G1_PIN 37
#define B1_PIN 38
#define R2_PIN 40
#define G2_PIN 41
#define B2_PIN 42
#define A_PIN 0
#define B_PIN 1
#define C_PIN 2
#define D_PIN 3
#define E_PIN -1
#define LAT_PIN 18
#define OE_PIN 16
#define CLK_PIN 15

#define CLOCK_R 231
#define CLOCK_G 103
#define CLOCK_B 3

class Matrix {
 private:
  std::unique_ptr<MatrixPanel_I2S_DMA> _dma_display;
  void setCenteredCursorPosition(const String& text);
  void drawPixel(int16_t x, int16_t y, uint16_t colour);

 public:
  Matrix();
  void println(const char* text, bool clear = true, uint16_t cursor_x = 0,
               uint16_t cursor_y = 0);
  void printClock(String time);
  void drawGif(GIFDRAW *pDraw);
};