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

#include "Matrix.hpp"

Matrix::Matrix() {
  Serial.println("Matrix: Initizalizing the matrix component...");
  // module configuration
  HUB75_I2S_CFG mxconfig(PANEL_RES_X,  // module width
                         PANEL_RES_Y,  // module height
                         PANEL_CHAIN   // chain length
  );
  mxconfig.gpio.r1 = R1_PIN;
  mxconfig.gpio.g1 = G1_PIN;
  mxconfig.gpio.b1 = B1_PIN;
  mxconfig.gpio.r2 = R2_PIN;
  mxconfig.gpio.g2 = G2_PIN;
  mxconfig.gpio.b2 = B2_PIN;
  mxconfig.gpio.a = A_PIN;
  mxconfig.gpio.b = B_PIN;
  mxconfig.gpio.c = C_PIN;
  mxconfig.gpio.d = D_PIN;
  // mxconfig.gpio.e = E_PIN;
  mxconfig.gpio.lat = LAT_PIN;
  mxconfig.gpio.oe = OE_PIN;
  mxconfig.gpio.clk = CLK_PIN;
  mxconfig.clkphase = false;
  // mxconfig.latch_blanking = 4;
  // mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  //  initialize the display
  _dma_display = std::make_unique<MatrixPanel_I2S_DMA>(mxconfig);
  _dma_display->begin();
  _dma_display->setBrightness8(90);  // 0-255
  _dma_display->clearScreen();
  Serial.println("Matrix: Initizalized the matrix component!");
}

void Matrix::setCenteredCursorPosition(const String& text) {
  int16_t x, y;
  int16_t x1, y1;
  uint16_t w, h;

  _dma_display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  // Center horizontally
  x = (PANEL_RES_X * PANEL_CHAIN - w) / 2;
  y = (PANEL_RES_Y - h) / 2 - y1;
  _dma_display->setCursor(x, y);
}

void Matrix::drawPixel(int16_t x, int16_t y, uint16_t colour) {
  _dma_display->drawPixel(x, y, colour);
}

void Matrix::println(const char* text, bool clear, uint16_t cursor_x,
                     uint16_t cursor_y) {
  if (clear) {
    _dma_display->clearScreen();
    _dma_display->setCursor(cursor_x, cursor_y);
  }
  _dma_display->println(text);
}

void Matrix::printClock(String time) {
  _dma_display->clearScreen();
  _dma_display->setFont(&FreeMonoBold18pt7b);
  setCenteredCursorPosition(time);
  _dma_display->setTextColor(_dma_display->color565(CLOCK_R, CLOCK_G, CLOCK_B));
  _dma_display->setTextSize(1);
  _dma_display->println(time);
}