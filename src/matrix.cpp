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

#include "matrix.hpp"

#define PANEL_RES_X 64  // vertical number of pixels per display
#define PANEL_RES_Y 32  // vertical number of pixels per display
#define PANEL_CHAIN 2   // number of displays

Matrix::Matrix() {
  // module configuration
  HUB75_I2S_CFG mxconfig(PANEL_RES_X,  // module width
                         PANEL_RES_Y,  // module height
                         PANEL_CHAIN   // chain length
  );
  // initialize the display
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90);  // 0-255
  dma_display->clearScreen();
}

void Matrix::drawPixel(int16_t x, int16_t y, uint16_t colour) {
  dma_display->drawPixel(x, y, colour);
}
