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

#include "application/Clock.hpp"
#include "display/Logo.hpp"

namespace display::matrix {

void Matrix::begin() {
  Serial.println("Matrix: Initializing the matrix component...");

  panel_width = panel_res_x * panel_count_x;
  panel_height = panel_res_y * panel_count_y;
  // Only a single row of panels is supported
  panel_chain = panel_count_x * panel_count_y;

  // module configuration
  HUB75_I2S_CFG mxconfig(panel_res_x,  // module width
                         panel_res_y,  // module height
                         panel_chain   // chain length
  );
  mxconfig.gpio.r1 = pins_r1;
  mxconfig.gpio.g1 = pins_g1;
  mxconfig.gpio.b1 = pins_b1;
  mxconfig.gpio.r2 = pins_r2;
  mxconfig.gpio.g2 = pins_g2;
  mxconfig.gpio.b2 = pins_b2;
  mxconfig.gpio.a = pins_a;
  mxconfig.gpio.b = pins_b;
  mxconfig.gpio.c = pins_c;
  mxconfig.gpio.d = pins_d;
  mxconfig.gpio.e = pins_e;
  mxconfig.gpio.lat = pins_lat;
  mxconfig.gpio.oe = pins_oe;
  mxconfig.gpio.clk = pins_clk;
  mxconfig.clkphase = false;
  // mxconfig.latch_blanking = 4;
  // mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  //  initialize the display
  dma_display = std::make_unique<MatrixPanel_I2S_DMA>(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(power ? brightness : 0);
  dma_display->clearScreen();
  Serial.println("Matrix: Initialized the matrix component!");
}

void Matrix::clearScreen() { dma_display->clearScreen(); }

/**
 * Sets the brightness (0-255), only applied to the panel while it is on
 */
void Matrix::setBrightness(uint8_t value) {
  brightness = value;
  if (power) dma_display->setBrightness8(brightness);
}

/**
 * Turns the panel on or off, the brightness is kept for turning it back on
 */
void Matrix::setPower(bool on) {
  power = on;
  if (power) {
    dma_display->setBrightness8(brightness);
  } else {
    dma_display->clearScreen();
    dma_display->setBrightness8(0);
  }
}

bool Matrix::isOn() { return power; }

void Matrix::setCenteredCursorPosition(const String &text) {
  int16_t x, y;
  int16_t x1, y1;
  uint16_t w, h;

  dma_display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  // Center horizontally
  x = (panel_width - w) / 2;
  y = (panel_height - h) / 2 - y1;
  dma_display->setCursor(x, y);
}

/**
 * Shows the logo centered on the matrix, cut off on smaller matrices
 */
void Matrix::printLogo() {
  int16_t offset_x = (static_cast<int16_t>(panel_width) - LOGO_WIDTH) / 2;
  int16_t offset_y = (static_cast<int16_t>(panel_height) - LOGO_HEIGHT) / 2;
  for (int16_t y = 0; y < LOGO_HEIGHT; y++) {
    for (int16_t x = 0; x < LOGO_WIDTH; x++) {
      drawPixel(offset_x + x, offset_y + y, esPINDMD_logo[y * LOGO_WIDTH + x]);
    }
  }
}

/**
 * Shows the progress of a firmware update as text and progress bar
 */
void Matrix::printUpdateProgress(uint8_t percent) {
  uint16_t colour = textColour();
  String text = "Updating " + String(percent) + "%";

  dma_display->clearScreen();
  dma_display->setFont(nullptr);
  dma_display->setTextSize(1);
  dma_display->setTextColor(colour);
  int16_t x1, y1;
  uint16_t w, h;
  dma_display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  dma_display->setCursor((panel_width - w) / 2, panel_height / 2 - h - 1);
  dma_display->print(text);

  // Progress bar below the text
  int16_t bar_x = 4;
  int16_t bar_y = panel_height / 2 + 2;
  int16_t bar_w = panel_width - 2 * bar_x;
  dma_display->drawRect(bar_x, bar_y, bar_w, 6, colour);
  dma_display->fillRect(bar_x + 1, bar_y + 1, (bar_w - 2) * percent / 100, 4,
                        colour);
}

/**
 * Shows up to four lines of text in the small default font, each line is
 * centered horizontally and the block is centered vertically
 */
void Matrix::printLines(std::initializer_list<String> lines) {
  // Height of the default font
  constexpr int16_t LINE_HEIGHT = 8;

  dma_display->clearScreen();
  dma_display->setFont(nullptr);
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setTextColor(textColour());

  int16_t y =
      (panel_height - static_cast<int16_t>(lines.size()) * LINE_HEIGHT) / 2;
  for (const String &line : lines) {
    int16_t x1, y1;
    uint16_t w, h;
    dma_display->getTextBounds(line, 0, 0, &x1, &y1, &w, &h);
    // Lines that are too wide start at the left edge and get cut off
    int16_t x = w < panel_width ? (panel_width - w) / 2 : 0;
    dma_display->setCursor(x, y);
    dma_display->print(line);
    y += LINE_HEIGHT;
  }
}

void Matrix::drawPixel(int16_t x, int16_t y, uint16_t colour) {
  dma_display->drawPixel(x, y, colour);
}

/**
 * Colour used for all text, configured by the clock colour
 */
uint16_t Matrix::textColour() {
  return dma_display->color565(application::clock::colour_r,
                               application::clock::colour_g,
                               application::clock::colour_b);
}

/**
 * Shows a single text centered on the matrix using the given font
 */
void Matrix::printCentered(const String &text, const GFXfont *font) {
  dma_display->clearScreen();
  dma_display->setFont(font);
  dma_display->setTextSize(1);
  dma_display->setTextColor(textColour());
  setCenteredCursorPosition(text);
  dma_display->print(text);
}

void Matrix::printClock(const String &time) {
  printCentered(time, &FreeMonoBold18pt7b);
}

void Matrix::printDate(const String &date) {
  printCentered(date, &FreeMonoBold9pt7b);
}

/**
 * Callback of the AnimatedGIF library drawing a single line of a frame
 */
void Matrix::drawGif(GIFDRAW *pDraw) {
  int16_t y = pDraw->iY + pDraw->y;
  if (y >= panel_height) return;

  // Clip lines wider than the matrix
  int16_t width = std::min<int16_t>(pDraw->iWidth, panel_width - pDraw->iX);
  uint8_t *pixels = pDraw->pPixels;
  uint16_t *palette = pDraw->pPalette;

  // Restore transparent pixels to the background colour
  if (pDraw->ucDisposalMethod == 2) {
    for (int16_t x = 0; x < width; x++) {
      if (pixels[x] == pDraw->ucTransparent) pixels[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }

  for (int16_t x = 0; x < width; x++) {
    // Transparent pixels keep the content of the previous frame
    if (pDraw->ucHasTransparency && pixels[x] == pDraw->ucTransparent) {
      continue;
    }
    drawPixel(pDraw->iX + x, y, palette[pixels[x]]);
  }
}

}  // namespace display::matrix
