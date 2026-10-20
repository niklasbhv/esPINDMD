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

#include <Arduino.h>

namespace storage::configuration {

constexpr const char* CONFIG_FILE_PATH = "/config.json";
// The configuration is written to this file first and then renamed,
// so a power loss while saving does not corrupt the configuration
constexpr const char* CONFIG_TMP_FILE_PATH = "/config.json.tmp";
// Location of the configuration in the internal flash (NVS)
constexpr const char* NVS_NAMESPACE = "espindmd";
constexpr const char* NVS_KEY = "config";

/**
 * The configuration class is used for reading and writing the
 * configuration. It is stored in the internal flash, so it is available
 * before and without the SD card. A config.json on the SD card is imported
 * on boot and overrides the stored values, which allows editing the
 * configuration on a computer.
 *
 * The stored values are described in Settings.hpp, missing values keep
 * their defaults.
 */
class Configuration {
 public:
  // Loads the configuration from the internal flash
  static bool load();
  // Imports the config.json from the SD card into the internal flash
  static bool importFromSd();
  // Saves the configuration to the internal flash and the SD card
  static bool save();
  // Disallow creating an instance of this object
  Configuration() = delete;
};

}  // namespace storage::configuration
