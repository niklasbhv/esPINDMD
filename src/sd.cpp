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

#include "sd.hpp"

// defines the SPI clock speed, this is optimized for stability
#define SPI_SPEED SD_SCK_MHZ(4)

// Chip select pin for the SD Card module
const uint8_t SD_CS_PIN = 10;

Sd::Sd() {
  Serial.println("Initializing the SD Card");
  Serial.println("\nSPI pins:\n");
  Serial.println("MISO: " + String(MISO));
  Serial.println("MOSI: " + String(MOSI));
  Serial.println("SCK:  " + String(SCK));
  Serial.println("SS:   " + String(SS));
#ifdef SDCARD_SS_PIN
  Serial.println("SDCARD_SS_PIN: " + String(SDCARD_SS_PIN));
#endif  // SDCARD_SS_PIN
  // try to initialize the sd card
  if (!sd.begin(SD_CS_PIN, SPI_SPEED)) {
    Serial.println("SD card initialization failed!");
  }
}