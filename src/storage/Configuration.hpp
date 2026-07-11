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
#include <WiFiManager.h>

#include "Sd.hpp"
#include "application/Clock.hpp"
#include "display/Matrix.hpp"
#include "network/Mqtt.hpp"
#include "network/Wifi.hpp"

namespace storage::configuration {

#define CONFIG_FILE_PATH "/config.json"

/**
 * Helper function used to retrieve values of the correct type from a JSON
 * We do not care, if the retreival fails as every value has a default
 */
template <typename V, typename T>
void get(const V& value, T& out) {
  JsonVariantConst v = value;
  if (v.is<T>()) {
    out = v.as<T>();
  }
}

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

  Configuration();
  void load();
  void loadDefault();
  void save();
};

}  // namespace storage::configuration
