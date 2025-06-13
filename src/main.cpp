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

#include <Arduino.h>

#include "Clock.hpp"
#include "Gif.hpp"
#include "Matrix.hpp"
#include "Sd.hpp"
#include "Wifi.hpp"

void setup() {
  // begin the serial console
  Serial.begin(115200);
  // setup the Wi-Fi configuration
  Wifi wifi;
  // setup the matrix configuration
  Matrix matrix;
  // setup the SD Card configuration
  Sd sd;
  // setup the clock
  Clock clock;
  // setup the gif class
  Gif gif(matrix);
}

void loop() {}