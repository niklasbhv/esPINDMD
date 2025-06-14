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

#include "Configuration.hpp"

#include <type_traits>

#include "ArduinoNvs.h"

Configuration::Configuration() { NVS.begin(); }

Configuration::~Configuration() { NVS.close(); }

int Configuration::readInt(String key) { return NVS.getInt(key); }

float Configuration::readFloat(String key) { return NVS.getFloat(key); }

String Configuration::readString(String key) { return NVS.getString(key); }

std::tuple<std::vector<uint8_t>, size_t> Configuration::readBytes(String key) {
  return make_tuple(NVS.getBlob(key), NVS.getBlobSize(key));
}

bool Configuration::writeInt(String key,
                             std::variant<uint8_t, int16_t, uint16_t, int32_t,
                                          uint32_t, int64_t, uint64_t>
                                 value) {
  if (std::holds_alternative<uint8_t>(value)) {
    return NVS.setInt(key, std::get<uint8_t>(value));
  } else if (std::holds_alternative<int16_t>(value)) {
    return NVS.setInt(key, std::get<int16_t>(value));
  } else if (std::holds_alternative<uint16_t>(value)) {
    return NVS.setInt(key, std::get<uint16_t>(value));
  } else if (std::holds_alternative<int32_t>(value)) {
    return NVS.setInt(key, std::get<int32_t>(value));
  } else if (std::holds_alternative<uint32_t>(value)) {
    return NVS.setInt(key, std::get<uint32_t>(value));
  } else if (std::holds_alternative<int64_t>(value)) {
    return NVS.setInt(key, std::get<int64_t>(value));
  } else if (std::holds_alternative<uint64_t>(value)) {
    return NVS.setInt(key, std::get<uint64_t>(value));
  } else {
    return false;
  }
}

bool Configuration::writeFloat(String key, float value) {
  return NVS.setFloat(key, value);
}

bool Configuration::writeString(String key, String value) {
  return NVS.setString(key, value);
}

bool Configuration::writeBytes(String key, std::vector<uint8_t> value) {
  return NVS.setBlob(key, value);
}