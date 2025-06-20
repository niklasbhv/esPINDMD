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

SystemController::SystemController() {
  // setup the Wi-Fi configuration
  Wifi _wifi;
  // setup the matrix configuration
  Matrix _matrix;
  // setup the SD Card configuration
  Sd _sd;
  // setup the clock
  Clock _clock;
  // setup the gif class
  Gif _gif;
  // setup the mqtt config
  Mqtt _mqtt("mqtt.test.org", 1883);
  // setup the cli
  Cli _cli;
};