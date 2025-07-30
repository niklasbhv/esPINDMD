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

#include "SystemController.hpp"

#define SHOW_CLOCK_MS 5000

SystemController::SystemController(){};

void SystemController::displayLogo() {
    // Draw image to screen
  int imgWidth = 128;
  int imgHeight = 32;

  for (int y = 0; y < imgHeight; y++) {
    for (int x = 0; x < imgWidth; x++) {
      uint16_t color = esPINDMD_logo[y * imgWidth + x];
      Matrix::drawPixel(x, y, color);
    }
  }
}

void SystemController::begin() {
  Serial.println("SystemController: Initializing the hardware components...");
  delay(5000);
  // setup the matrix configuration
  Matrix::begin();
  Matrix::println("Matrix Initialized");
  // setup the SD Card configuration
  _sd = std::make_unique<Sd>();
  Matrix::println("SD Card Initialized");
  // setup the Wi-Fi configuration
  _wifi = std::make_unique<Wifi>();
  Matrix::println("Wi-Fi Initialized");
  if (WiFi.status() == WL_CONNECTED) {
    Matrix::println(WiFi.localIP().toString().c_str());
  }
  Serial.println("SystemController: Initialized the hardware components!");
  Serial.println("SystemController: Initializing the software components...");
  // setup the clock
  _clock = std::make_unique<Clock>();
  // setup the gif library
  _gif.begin(LITTLE_ENDIAN_PIXELS);
  // setup the mqtt config
  // _mqtt = std::make_unique<Mqtt>(MQTT_SERVER, MQTT_SERVER_PORT);
  // setup the cli
  // _cli = std::make_unique<Cli>();
  Serial.println("SystemController: Initialized the software components!");
  displayLogo();
  delay(5000);
}

bool SystemController::displayGif(String& filename) {
  if (!_gif.open(filename.c_str(), Sd::openGifFile, Sd::closeGifFile,
                 Sd::readGifFile, Sd::seekGifFile, Matrix::drawGif)) {
    Serial.println("SystemController: Failed to read GIF!");
    return false;
  } else {
    dma_display->clearScreen();
    while (_gif.playFrame(true, NULL)) {
    }
    _gif.close();
  }
  return true;
}

void SystemController::loop() {
  Matrix::printClock(_clock->dateTime("H:i"));
  delay(SHOW_CLOCK_MS);
  String filename;
  if (!_sd->next(filename)) {
    Serial.println(
        "SystemController: Iterater exhausted, resetting the iterator!");
    _sd->resetIterator();
  } else {
    displayGif(filename);
  }
}
