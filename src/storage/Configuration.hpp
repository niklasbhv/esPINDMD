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

#include <variant>
#include <vector>

/**
 * The configuration class is used for reading
 * and writing the configuration from the non
 * volatile storage of the esp32
 */
class Configuration {
 private:
  ~Configuration();

 public:
  Configuration();
  int readInt(String key);
  float readFloat(String key);
  String readString(String key);
  std::tuple<std::vector<uint8_t>, size_t> readBytes(String key);
  bool writeInt(String key, std::variant<uint8_t, int16_t, uint16_t, int32_t,
                                         uint32_t, int64_t, uint64_t>
                                value);
  bool writeFloat(String key, float value);
  bool writeString(String key, String value);
  bool writeBytes(String key, std::vector<uint8_t> value);
};