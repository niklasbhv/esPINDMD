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

void Matrix::begin() {
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
  dma_display = std::make_unique<MatrixPanel_I2S_DMA>(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90);  // 0-255
  dma_display->clearScreen();
  Serial.println("Matrix: Initizalized the matrix component!");
}

void Matrix::clearScreen() {
  dma_display->clearScreen();
}

void Matrix::setCenteredCursorPosition(const String &text) {
  int16_t x, y;
  int16_t x1, y1;
  uint16_t w, h;

  dma_display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  // Center horizontally
  x = (PANEL_WIDTH - w) / 2;
  y = (PANEL_HEIGHT - h) / 2 - y1;
  dma_display->setCursor(x, y);
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

void Matrix::drawGif(GIFDRAW *pDraw) {
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > PANEL_WIDTH) iWidth = PANEL_WIDTH;

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
