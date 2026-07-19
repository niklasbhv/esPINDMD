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
SystemController::SystemController() {};

/**
 * Function used to initialize the SystemController
 */
void SystemController::begin() {
    // setup the SD Card configuration
  _sd = std::make_unique<storage::sd::Sd>();
  //delay(1000);
  Serial.println("SystemController: Loading configuration...");
  if (storage::configuration::Configuration::load()) {
      Serial.println("SystemController: Configuration loaded");
  }
  else {
      Serial.println("SystemController: No configuration found! Loading defaults");
  }

  Serial.println("SystemController: Initializing the hardware components...");
  delay(5000);
  // setup the matrix configuration
  display::matrix::Matrix::begin();
  // setup the Wi-Fi configuration
  _wifi = std::make_unique<network::wifi::Wifi>();
  if (WiFi.status() == WL_CONNECTED) {
    display::matrix::Matrix::println(WiFi.localIP().toString().c_str());
  }
  Serial.println("SystemController: Initialized the hardware components!");
  Serial.println("SystemController: Initializing the software components...");
  // setup the clock
  _clock = std::make_unique<application::clock::Clock>();
  // setup the gif library
  _gif.begin(LITTLE_ENDIAN_PIXELS);
  Serial.println("SystemController: Initialized the software components!");
  display::matrix::Matrix::printLogo();
  delay(5000);
}

/**
 * Function used for loading and displaying the given GIF file
 */
bool SystemController::displayGif(String& filename) {
  if (!_gif.open(filename.c_str(), storage::sd::Sd::openGifFile,
                 storage::sd::Sd::closeGifFile, storage::sd::Sd::readGifFile,
                 storage::sd::Sd::seekGifFile,
                 display::matrix::Matrix::drawGif)) {
    Serial.println("SystemController: Failed to read GIF!");
    return false;
  } else {
    display::matrix::Matrix::clearScreen();
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
  display::matrix::Matrix::printClock(_clock->dateTime("H:i"));
  delay(SHOW_APPLICATION_MS);
  display::matrix::Matrix::printDate(_clock->dateTime("d.M.Y"));
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
