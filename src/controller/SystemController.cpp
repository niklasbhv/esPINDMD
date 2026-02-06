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

/**
 * Constructor of the SystemController class
 */
SystemController::SystemController(){};

/**
 * Function used for displaying the esPINDMD logo
 */
void SystemController::displayLogo() {
  for (int y = 0; y < PANEL_HEIGHT; y++) {
    for (int x = 0; x < PANEL_WIDTH; x++) {
      uint16_t color = esPINDMD_logo[y * PANEL_WIDTH + x];
      Matrix::drawPixel(x, y, color);
    }
  }
}

/**
 * Function used to initialize the SystemController
 */
void SystemController::begin() {
  Serial.println("SystemController: Initializing the hardware components...");
  delay(5000);
  // setup the matrix configuration
  Matrix::begin();
  Matrix::println("Matrix Initialized");
  // setup the SD Card configuration
  _sd = std::make_unique<Sd>();
  Matrix::println("SD Card Initialized");
  Matrix::println("Loading Configuration...");
  _config = std::make_unique<Configuration>();
  Matrix::println("Configuration loaded!");
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
  Serial.println("SystemController: Initialized the software components!");
  displayLogo();
  delay(5000);
}

/**
 * Function used for loading and displaying the given GIF file
 */
bool SystemController::displayGif(String& filename) {
  if (!_gif.open(filename.c_str(), Sd::openGifFile, Sd::closeGifFile,
                 Sd::readGifFile, Sd::seekGifFile, Matrix::drawGif)) {
    Serial.println("SystemController: Failed to read GIF!");
    return false;
  } else {
    Matrix::clearScreen();
    while (_gif.playFrame(true, NULL)) {
    }
    _gif.close();
  }
  return true;
}

/**
 * Function used for running the application parts of the device
 */
void SystemController::applicationLoop() {
  Matrix::printClock(_clock->dateTime("H:i"));
  delay(SHOW_APPLICATION_MS);
  Matrix::printClock(_clock->dateTime("d.M.Y"));
  delay(SHOW_APPLICATION_MS);
  String filename;
  if (!_sd->next(filename)) {
    Serial.println(
        "SystemController: Iterater exhausted, resetting the iterator!");
    _sd->resetIterator();
  } else {
    displayGif(filename);
  }
}
/**
 * Function used for running the system parts of the device
 */
void SystemController::systemLoop() {}
