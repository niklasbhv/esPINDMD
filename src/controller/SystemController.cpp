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

#define GIF_ROOT_PATH "/gif"

SystemController::SystemController(){};

void SystemController::begin() {
  Serial.println("SystemController: Initializing the hardware components...");
  // setup the matrix configuration
  _matrix = std::make_unique<Matrix>();
  _matrix->println("Matrix Initialized");
  // setup the SD Card configuration
  _sd = std::make_unique<Sd>();
  _matrix->println("SD Card Initialized");
  // setup the Wi-Fi configuration
  _wifi = std::make_unique<Wifi>();
  _matrix->println("Wi-Fi Initialized");
  if (WiFi.status() == WL_CONNECTED) {
    _matrix->println(WiFi.localIP().toString().c_str());
  }
  Serial.println("SystemController: Initialized the hardware components!");
  Serial.println("SystemController: Initializing the software components...");
  // setup the clock
  _clock = std::make_unique<Clock>();
  // setup the mqtt config
  _mqtt = std::make_unique<Mqtt>("mqtt.test.org", 1883);
  // setup the cli
  _cli = std::make_unique<Cli>();
  // setup the file iterator
  //_sequentialIterator = std::make_unique<SequentialIterator>(,GIF_ROOT_PATH);
  Serial.println("SystemController: Initialized the software components!");
}

void SystemController::loop() {
  _matrix->printClock(_clock->dateTime("H:i"));
  delay(SHOW_CLOCK_MS);
  char* filename;
  size_t filename_size;
  //_sequentialIterator->next(filename, filename_size);
}