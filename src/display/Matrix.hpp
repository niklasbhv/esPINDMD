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

#include <AnimatedGIF.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <SdFat.h>

#include "Fonts/FreeMonoBold18pt7b.h"
#include "Fonts/FreeMonoBold9pt7b.h"
#include "application/Clock.hpp"
#include "display/Logo.hpp"

namespace display::matrix {

inline uint8_t panel_res_x = 64;   // horizontal number of pixels per display
inline uint8_t panel_res_y = 32;   // vertical number of pixels per display
inline uint8_t panel_count_x = 2;  // number of displays on the x axis
inline uint8_t panel_count_y = 1;  // Number of displays on the y axis

inline uint16_t panel_width = 0;
inline uint16_t panel_height = 0;
inline uint16_t panel_chain = 0;

inline int8_t pins_r1 = 36;
inline int8_t pins_g1 = 37;
inline int8_t pins_b1 = 38;
inline int8_t pins_r2 = 40;
inline int8_t pins_g2 = 41;
inline int8_t pins_b2 = 42;
inline int8_t pins_a = 0;
inline int8_t pins_b = 1;
inline int8_t pins_c = 2;
inline int8_t pins_d = 3;
inline int8_t pins_e = -1;
inline int8_t pins_lat = 18;
inline int8_t pins_oe = 16;
inline int8_t pins_clk = 15;

// Global instance of the dma_display
// Needed because of a limitation of the AnimationGIF library
inline std::unique_ptr<MatrixPanel_I2S_DMA> dma_display;

class Matrix {
 private:
  static void setCenteredCursorPosition(const String& text);

 public:
  static void begin();
  static void clearScreen();
  static void println(const char* text, bool clear = true,
                      uint16_t cursor_x = 0, uint16_t cursor_y = 0);
  static void printClock(String time);
  static void printDate(String date);
  static void printLogo();
  static void drawPixel(int16_t x, int16_t y, uint16_t colour);
  static void drawGif(GIFDRAW* pDraw);

  // Disallow creating an instance of this object
  Matrix() = delete;
};

}  // namespace display::matrix
