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

namespace display::matrix {

void Matrix::begin() {
  Serial.println("Matrix: Initizalizing the matrix component...");

  panel_width = panel_res_x * panel_count_x;
  panel_height = panel_res_y * panel_count_y;
  panel_chain = panel_count_x + panel_count_y - 1;

  // module configuration
  HUB75_I2S_CFG mxconfig(panel_res_x,   // module width
                         panel_res_y,  // module height
                         panel_chain    // chain length
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
  // mxconfig.gpio.e = pins_e;
  mxconfig.gpio.lat = pins_lat;
  mxconfig.gpio.oe = pins_oe;
  mxconfig.gpio.clk = pins_clk;
  mxconfig.clkphase = false;
  // mxconfig.latch_blanking = 4;
  // mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  //  initialize the display
  dma_display = std::make_unique<MatrixPanel_I2S_DMA>(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90);  // 0-255
  dma_display->clearScreen();
  Serial.println("Matrix: Initizalized the matrix component!");
}

void Matrix::clearScreen() { dma_display->clearScreen(); }

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

void Matrix::printLogo() {
  for (int y = 0; y < panel_height; y++) {
    for (int x = 0; x < panel_width; x++) {
      uint16_t color = esPINDMD_logo[y * panel_width + x];
      Matrix::drawPixel(x, y, color);
    }
  }
}

void Matrix::drawPixel(int16_t x, int16_t y, uint16_t colour) {
  dma_display->drawPixel(x, y, colour);
}

void Matrix::println(const char *text, bool clear, uint16_t cursor_x,
                     uint16_t cursor_y) {
  if (clear) {
    dma_display->clearScreen();
    dma_display->setCursor(cursor_x, cursor_y);
  }
  dma_display->println(text);
}

void Matrix::printClock(String time) {
  dma_display->clearScreen();
  dma_display->setFont(&FreeMonoBold18pt7b);
  setCenteredCursorPosition(time);
  dma_display->setTextColor(dma_display->color565(CLOCK_R, CLOCK_G, CLOCK_B));
  dma_display->setTextSize(1);
  dma_display->println(time);
}

void Matrix::printDate(String date) {
  dma_display->clearScreen();
  dma_display->setFont(&FreeMonoBold9pt7b);
  setCenteredCursorPosition(date);
  dma_display->setTextColor(dma_display->color565(CLOCK_R, CLOCK_G, CLOCK_B));
  dma_display->setTextSize(1);
  dma_display->println(date);
}

void Matrix::drawGif(GIFDRAW *pDraw) {
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > panel_width) iWidth = panel_width;

  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y;  // current line

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2)  // restore to background color
  {
    for (x = 0; x < iWidth; x++) {
      if (s[x] == pDraw->ucTransparent) s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency)  // if transparency used
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + pDraw->iWidth;
    x = 0;
    iCount = 0;  // count non-transparent pixels
    while (x < pDraw->iWidth) {
      c = ucTransparent - 1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent)  // done, stop
        {
          s--;  // back up to treat it like transparent
        } else  // opaque
        {
          *d++ = usPalette[c];
          iCount++;
        }
      }  // while looking for opaque pixels
      if (iCount)  // any opaque pixels?
      {
        for (int xOffset = 0; xOffset < iCount; xOffset++) {
          drawPixel(x + xOffset, y,
                    usTemp[xOffset]);  // 565 Color Format
        }
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent)
          iCount++;
        else
          s--;
      }
      if (iCount) {
        x += iCount;  // skip these
        iCount = 0;
      }
    }
  } else  // does not have transparency
  {
    s = pDraw->pPixels;
    // Translate the 8-bit pixels through the RGB565 palette (already byte
    // reversed)
    for (x = 0; x < pDraw->iWidth; x++) {
      drawPixel(x, y, usPalette[*s++]);  // color 565
    }
  }
}

}  // namespace display::matrix
