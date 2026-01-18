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

#include <ArduinoJson.h>
#include "Sd.hpp"

#define CONFIG_FILE_PATH "/config.json"

/**
 * The configuration class is used for reading
 * and writing the configuration from the SD-Card.
 * 
 * If no configuration is found, a new default one 
 * is generated.
 */
class Configuration {
 public:
  // Configuration variables
  // Common variables
  char deviceName[32];

  // clock variables
  uint8_t clockColourR;
  uint8_t clockColourG;
  uint8_t clockColourB;

  // Mqtt variables
  bool mqttEnabled;
  char mqttServer[32];
  uint16_t mqttPort;
  char mqttCredentialsUsername[32];
  char mqttCredentialsPassword[32];

  Configuration();
  void load();
  void loadDefault();
  void save();
};