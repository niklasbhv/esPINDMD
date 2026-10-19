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
  Serial.println("SystemController: Loading configuration...");
  if (storage::configuration::Configuration::load()) {
    Serial.println("SystemController: Configuration loaded");
  } else {
    Serial.println(
        "SystemController: No configuration found! Loading defaults");
  }

  Serial.println("SystemController: Initializing the hardware components...");
  delay(2000);
  // setup the matrix configuration
  display::matrix::Matrix::begin();
  display::matrix::Matrix::printLogo();
  // setup the Wi-Fi configuration
  _wifi = std::make_unique<network::wifi::Wifi>();
  if (WiFi.status() == WL_CONNECTED) {
    // display::matrix::Matrix::println(WiFi.localIP().toString().c_str());
  }
  Serial.println("SystemController: Initialized the hardware components!");
  Serial.println("SystemController: Initializing the software components...");
  // setup MQTT if enabled
  if (network::mqtt::enable) {
    _mqtt = std::make_unique<network::mqtt::Mqtt>(
        [this](const network::mqtt::LightCommand& command) {
          if (command.brightness &&
              *command.brightness != display::matrix::brightness) {
            display::matrix::Matrix::setBrightness(*command.brightness);
            // Saving is deferred to bundle changes, e.g. from a slider
            _config_changed_at = millis();
            _config_dirty = true;
          }
          if (command.on) {
            display::matrix::Matrix::setPower(*command.on);
          }
        },
        []() {
          return network::mqtt::LightState{display::matrix::Matrix::isOn(),
                                           display::matrix::brightness};
        });
    _mqtt->begin();
  }

  // setup the clock
  _clock = std::make_unique<application::clock::Clock>();
  // setup the gif library
  _gif.begin(LITTLE_ENDIAN_PIXELS);
  Serial.println("SystemController: Initialized the software components!");
  delay(2000);
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
    // Keep the system parts running between frames and stop playback
    // as soon as the matrix gets turned off
    while (display::matrix::Matrix::isOn() && _gif.playFrame(true, NULL)) {
      systemLoop();
    }
    _gif.close();
  }
  return true;
}

/**
 * Function used to wait while keeping the system parts running,
 * returns early if the matrix gets turned off
 */
void SystemController::wait(uint32_t ms) {
  uint32_t start = millis();
  while (millis() - start < ms && display::matrix::Matrix::isOn()) {
    systemLoop();
    delay(10);
  }
}

/**
 * Function used for running the application parts of the device
 */
void SystemController::applicationLoop() {
  if (!display::matrix::Matrix::isOn()) return;

  display::matrix::Matrix::printClock(_clock->dateTime("H:i"));
  wait(SHOW_APPLICATION_MS);
  if (!display::matrix::Matrix::isOn()) return;
  display::matrix::Matrix::printDate(_clock->dateTime("d.M.Y"));
  wait(SHOW_APPLICATION_MS);
  if (!display::matrix::Matrix::isOn()) return;
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
void SystemController::systemLoop() {
  if (_mqtt) _mqtt->loop();

  if (_config_dirty && millis() - _config_changed_at >= SAVE_DELAY_MS) {
    _config_dirty = false;
    storage::configuration::Configuration::save();
  }
}
