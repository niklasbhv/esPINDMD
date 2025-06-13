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

class Matrix {
 private:
  MatrixPanel_I2S_DMA *_dma_display = nullptr;

 public:
  Matrix();
  void drawPixel(int16_t x, int16_t y, uint16_t colour);
};