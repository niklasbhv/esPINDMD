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

#include <ArduinoJson.h>

#include "Sd.hpp"
#include "Settings.hpp"

namespace storage::configuration {

/**
 * Function used to look up the value of a setting in the JSON document,
 * returns a null variant if any part of the path is missing
 */
static JsonVariantConst find(const JsonDocument& doc, const char* path) {
  JsonVariantConst node = doc.as<JsonVariantConst>();
  std::string segment;
  for (const char* c = path;; c++) {
    if (*c == '/' || *c == '\0') {
      node = node[segment];
      segment.clear();
      if (*c == '\0' || node.isNull()) return node;
    } else {
      segment += *c;
    }
  }
}

/**
 * Function used to create the location of a setting in the JSON document,
 * missing objects on the path are created
 */
static JsonVariant create(JsonDocument& doc, const char* path) {
  JsonObject node = doc.as<JsonObject>();
  std::string segment;
  for (const char* c = path; *c != '\0'; c++) {
    if (*c == '/') {
      JsonObject child = node[segment];
      if (child.isNull()) child = node[segment].to<JsonObject>();
      node = child;
      segment.clear();
    } else {
      segment += *c;
    }
  }
  return node[segment];
}

bool Configuration::load() {
  Serial.println("Configuration: Loading configuration...");
  // If saving was interrupted between removing the old and renaming the
  // new file, only the temporary file is left
  if (!storage::sd::Sd::exists(CONFIG_FILE_PATH) &&
      storage::sd::Sd::exists(CONFIG_TMP_FILE_PATH)) {
    Serial.println("Configuration: Restoring interrupted save");
    storage::sd::Sd::replaceFile(CONFIG_TMP_FILE_PATH, CONFIG_FILE_PATH);
  }
  FsFile config_file;
  if (!storage::sd::Sd::openFile(CONFIG_FILE_PATH, config_file)) {
    Serial.println("Configuration: Failed to open configuration file");
    return false;
  }
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, config_file);
  storage::sd::Sd::closeFile(config_file);

  if (err) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(err.c_str());
    return false;
  }

  // Missing, invalid or out of range values keep their defaults
  for (size_t i = 0; i < settings::count; i++) {
    const settings::Setting& setting = settings::all[i];
    JsonVariantConst value = find(doc, setting.path);
    if (value.isNull()) continue;
    std::visit(
        [&value, &setting](auto* target) {
          using T = std::remove_pointer_t<decltype(target)>;
          if (!value.is<T>()) {
            Serial.printf("Configuration: Ignoring %s, invalid type\r\n",
                          setting.path);
            return;
          }
          T result = value.as<T>();
          int32_t checked = 0;
          if constexpr (std::is_same_v<T, std::string>) {
            checked = result.length();
          } else {
            checked = static_cast<int32_t>(result);
          }
          if (!setting.accepts(checked)) {
            Serial.printf("Configuration: Ignoring %s, allowed is %s\r\n",
                          setting.path, setting.range().c_str());
            return;
          }
          *target = result;
        },
        setting.value);
  }
  return true;
}

bool Configuration::save() {
  Serial.println("Configuration: Saving configuration...");
  JsonDocument doc;
  doc.to<JsonObject>();
  for (size_t i = 0; i < settings::count; i++) {
    const settings::Setting& setting = settings::all[i];
    JsonVariant target = create(doc, setting.path);
    std::visit([&target](auto* value) { target.set(*value); }, setting.value);
  }

  FsFile config_file;
  if (!storage::sd::Sd::openFileForWrite(CONFIG_TMP_FILE_PATH, config_file)) {
    Serial.println("Configuration: Failed to open configuration file");
    return false;
  }
  size_t written = serializeJsonPretty(doc, config_file);
  // Closing flushes the file, a failure means the data is not on the card
  if (!storage::sd::Sd::closeFile(config_file) || written == 0) {
    Serial.println("Configuration: Failed to write configuration to SD");
    return false;
  }
  if (!storage::sd::Sd::replaceFile(CONFIG_TMP_FILE_PATH, CONFIG_FILE_PATH)) {
    Serial.println("Configuration: Failed to replace configuration file");
    return false;
  }

  Serial.println("Configuration: Saved configuration!");
  return true;
}

}  // namespace storage::configuration
