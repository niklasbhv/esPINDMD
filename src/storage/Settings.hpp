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

#include <string>
#include <variant>

namespace storage::settings {

/**
 * Flags describing how a setting is handled
 */
enum Flags : uint8_t {
  NONE = 0,
  // Only takes effect after a reboot
  REBOOT = 1 << 0,
  // Value is not shown, e.g. passwords
  SECRET = 1 << 1,
  // Not shown in the CLI, only stored in the configuration file
  HIDDEN = 1 << 2,
};

using Value =
    std::variant<bool*, uint8_t*, int8_t*, uint16_t*, std::string*>;

/**
 * A single setting of the device. The path corresponds to the location
 * of the value in the configuration file, e.g. "display/matrix/brightness"
 * is stored as {"display": {"matrix": {"brightness": 90}}}.
 */
struct Setting {
  const char* path;
  Value value;
  // Allowed range for numbers, maximum length for strings (0 = unlimited)
  int32_t min;
  int32_t max;
  uint8_t flags;
  const char* description;

  // Name of the setting, the last segment of the path
  const char* name() const;
  bool hasFlag(Flags flag) const;
  // Value formatted for displaying, secrets are masked
  std::string toString() const;
  // Allowed values formatted for displaying, e.g. "0-255"
  std::string range() const;
  // Parses and validates the input, the value is only changed on success
  bool parse(const std::string& input, std::string& error);
};

// All settings of the device, in the order of the configuration file
extern Setting all[];
extern const size_t count;

}  // namespace storage::settings
