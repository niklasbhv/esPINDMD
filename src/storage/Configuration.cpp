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
#include <Preferences.h>

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
  // Converting the member proxy directly would return an unbound variant
  // for a missing key, so the value could never be set
  return node[segment].to<JsonVariant>();
}

/**
 * Function used to apply the values of a JSON document to the settings.
 * Missing, invalid or out of range values keep their current value.
 */
static void fromJson(const JsonDocument& doc) {
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
}

/**
 * Function used to write all settings into a JSON document
 *
 * @return false if a setting could not be written, e.g. out of memory
 */
static bool toJson(JsonDocument& doc) {
  doc.to<JsonObject>();
  bool ok = true;
  for (size_t i = 0; i < settings::count; i++) {
    const settings::Setting& setting = settings::all[i];
    JsonVariant target = create(doc, setting.path);
    std::visit([&target, &ok](auto* value) { ok = target.set(*value) && ok; },
               setting.value);
  }
  return ok && !doc.overflowed();
}

/**
 * Function used to serialize all settings into a compact JSON string
 *
 * @return an empty string if the settings could not be serialized
 */
static std::string serialize() {
  JsonDocument doc;
  if (!toJson(doc)) return std::string();
  std::string json;
  serializeJson(doc, json);
  return json;
}

static bool saveToNvs(const std::string& json) {
  Preferences preferences;
  if (!preferences.begin(NVS_NAMESPACE, false)) {
    Serial.println("Configuration: Failed to open the internal flash");
    return false;
  }
  bool ok = preferences.putString(NVS_KEY, json.c_str()) == json.length();
  preferences.end();
  if (!ok) Serial.println("Configuration: Failed to write the internal flash");
  return ok;
}

static bool saveToSd(const JsonDocument& doc) {
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
  return true;
}

bool Configuration::load() {
  Serial.println("Configuration: Loading configuration...");
  Preferences preferences;
  // Opening read only fails if nothing was stored yet
  if (!preferences.begin(NVS_NAMESPACE, true)) {
    Serial.println("Configuration: Nothing stored yet, using defaults");
    return false;
  }
  String json =
      preferences.isKey(NVS_KEY) ? preferences.getString(NVS_KEY) : String();
  preferences.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    Serial.printf("Configuration: Invalid stored configuration: %s\r\n",
                  err.c_str());
    return false;
  }
  fromJson(doc);
  return true;
}

bool Configuration::importFromSd() {
  if (!storage::sd::Sd::available()) return false;

  // If saving was interrupted between removing the old and renaming the
  // new file, only the temporary file is left
  if (!storage::sd::Sd::exists(CONFIG_FILE_PATH) &&
      storage::sd::Sd::exists(CONFIG_TMP_FILE_PATH)) {
    Serial.println("Configuration: Restoring interrupted save");
    storage::sd::Sd::replaceFile(CONFIG_TMP_FILE_PATH, CONFIG_FILE_PATH);
  }
  FsFile config_file;
  if (!storage::sd::Sd::openFile(CONFIG_FILE_PATH, config_file)) {
    Serial.println("Configuration: No configuration file on the SD card");
    return false;
  }
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, config_file);
  storage::sd::Sd::closeFile(config_file);
  if (err) {
    Serial.printf("Configuration: Invalid configuration file: %s\r\n",
                  err.c_str());
    return false;
  }

  std::string before = serialize();
  fromJson(doc);
  std::string after = serialize();
  // Only write the internal flash if something changed
  if (!after.empty() && before != after) {
    Serial.println("Configuration: Imported the configuration file");
    saveToNvs(after);
  }
  return true;
}

bool Configuration::save() {
  Serial.println("Configuration: Saving configuration...");
  JsonDocument doc;
  // An incomplete document would replace the stored values with defaults
  if (!toJson(doc)) {
    Serial.println("Configuration: Failed to serialize, not saving");
    return false;
  }
  std::string json;
  serializeJson(doc, json);

  bool ok = saveToNvs(json);
  // The configuration file is kept in sync, as it is imported on boot
  if (storage::sd::Sd::available()) ok = saveToSd(doc) && ok;
  if (ok) Serial.println("Configuration: Saved configuration!");
  return ok;
}

}  // namespace storage::configuration
