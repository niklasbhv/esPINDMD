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

namespace storage::configuration {

void loadWifiPassword() {}

void loadMqttPassword() {}

boolean Configuration::load() {
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

  // Clock values
  get(doc["application"]["clock"]["colour"]["r"], application::clock::colour_r);
  get(doc["application"]["clock"]["colour"]["g"], application::clock::colour_g);
  get(doc["application"]["clock"]["colour"]["b"], application::clock::colour_b);

  get(doc["application"]["clock"]["ntp"]["server"],
      application::clock::ntp_server);
  get(doc["application"]["clock"]["ntp"]["timezone"],
      application::clock::ntp_timezone);
  get(doc["application"]["clock"]["ntp"]["sync"]["timeout"],
      application::clock::ntp_sync_timeout);

  // Display configuration
  // Matrix configuration

  get(doc["display"]["matrix"]["panel"]["resolution"]["x"],
      display::matrix::panel_res_x);
  get(doc["display"]["matrix"]["panel"]["resolution"]["y"],
      display::matrix::panel_res_y);
  get(doc["display"]["matrix"]["panel"]["count"]["x"],
      display::matrix::panel_count_x);
  get(doc["display"]["matrix"]["panel"]["count"]["y"],
      display::matrix::panel_count_y);

  get(doc["display"]["matrix"]["pins"]["r1"], display::matrix::pins_r1);
  get(doc["display"]["matrix"]["pins"]["g1"], display::matrix::pins_g1);
  get(doc["display"]["matrix"]["pins"]["b1"], display::matrix::pins_b1);

  get(doc["display"]["matrix"]["pins"]["r2"], display::matrix::pins_r2);
  get(doc["display"]["matrix"]["pins"]["g2"], display::matrix::pins_g2);
  get(doc["display"]["matrix"]["pins"]["b2"], display::matrix::pins_b2);

  get(doc["display"]["matrix"]["pins"]["a"], display::matrix::pins_a);
  get(doc["display"]["matrix"]["pins"]["b"], display::matrix::pins_b);
  get(doc["display"]["matrix"]["pins"]["c"], display::matrix::pins_c);
  get(doc["display"]["matrix"]["pins"]["d"], display::matrix::pins_d);
  get(doc["display"]["matrix"]["pins"]["e"], display::matrix::pins_e);

  get(doc["display"]["matrix"]["pins"]["lat"], display::matrix::pins_lat);
  get(doc["display"]["matrix"]["pins"]["oe"], display::matrix::pins_oe);
  get(doc["display"]["matrix"]["pins"]["clk"], display::matrix::pins_clk);
  get(doc["display"]["matrix"]["brightness"], display::matrix::brightness);

  // Mqtt values
  get(doc["network"]["mqtt"]["enable"], network::mqtt::enable);
  get(doc["network"]["mqtt"]["server"], network::mqtt::server);
  get(doc["network"]["mqtt"]["port"], network::mqtt::port);

  get(doc["network"]["mqtt"]["credentials"]["username"],
      network::mqtt::username);
  get(doc["network"]["mqtt"]["credentials"]["password"],
      network::mqtt::password);
  get(doc["network"]["mqtt"]["client"]["id"], network::mqtt::client_id);

  // Storage configuration
  // SD configuration
  get(doc["storage"]["sd"]["pins"]["mosi"], storage::sd::pins_mosi);
  get(doc["storage"]["sd"]["pins"]["miso"], storage::sd::pins_miso);
  get(doc["storage"]["sd"]["pins"]["clk"], storage::sd::pins_clk);
  get(doc["storage"]["sd"]["pins"]["cs"], storage::sd::pins_cs);

  // Update configuration
  get(doc["update"]["url"], update::ota::manifest_url);
  get(doc["update"]["interval"], update::ota::check_interval);

  return true;
}

boolean Configuration::save() {
  Serial.println("Configuration: Saving configuration...");
  JsonDocument doc;

  // Application configuration
  // Clock configuration
  doc["application"]["clock"]["colour"]["r"] = application::clock::colour_r;
  doc["application"]["clock"]["colour"]["g"] = application::clock::colour_g;
  doc["application"]["clock"]["colour"]["b"] = application::clock::colour_b;
  doc["application"]["clock"]["ntp"]["server"] = application::clock::ntp_server;
  doc["application"]["clock"]["ntp"]["timezone"] =
      application::clock::ntp_timezone;
  doc["application"]["clock"]["ntp"]["sync"]["timeout"] =
      application::clock::ntp_sync_timeout;

  // Display configuration
  // Matrix configuration
  doc["display"]["matrix"]["panel"]["resolution"]["x"] =
      display::matrix::panel_res_x;
  doc["display"]["matrix"]["panel"]["resolution"]["y"] =
      display::matrix::panel_res_y;
  doc["display"]["matrix"]["panel"]["count"]["x"] =
      display::matrix::panel_count_x;
  doc["display"]["matrix"]["panel"]["count"]["y"] =
      display::matrix::panel_count_y;

  doc["display"]["matrix"]["pins"]["r1"] = display::matrix::pins_r1;
  doc["display"]["matrix"]["pins"]["g1"] = display::matrix::pins_g1;
  doc["display"]["matrix"]["pins"]["b1"] = display::matrix::pins_b1;

  doc["display"]["matrix"]["pins"]["r2"] = display::matrix::pins_r2;
  doc["display"]["matrix"]["pins"]["g2"] = display::matrix::pins_g2;
  doc["display"]["matrix"]["pins"]["b2"] = display::matrix::pins_b2;

  doc["display"]["matrix"]["pins"]["a"] = display::matrix::pins_a;
  doc["display"]["matrix"]["pins"]["b"] = display::matrix::pins_b;
  doc["display"]["matrix"]["pins"]["c"] = display::matrix::pins_c;
  doc["display"]["matrix"]["pins"]["d"] = display::matrix::pins_d;
  doc["display"]["matrix"]["pins"]["e"] = display::matrix::pins_e;

  doc["display"]["matrix"]["pins"]["lat"] = display::matrix::pins_lat;
  doc["display"]["matrix"]["pins"]["oe"] = display::matrix::pins_oe;
  doc["display"]["matrix"]["pins"]["clk"] = display::matrix::pins_clk;
  doc["display"]["matrix"]["brightness"] = display::matrix::brightness;

  // Network configuration
  // MQTT configuration
  doc["network"]["mqtt"]["enable"] = network::mqtt::enable;
  doc["network"]["mqtt"]["server"] = network::mqtt::server;
  doc["network"]["mqtt"]["port"] = network::mqtt::port;
  doc["network"]["mqtt"]["credentials"]["username"] = network::mqtt::username;
  doc["network"]["mqtt"]["credentials"]["password"] = network::mqtt::password;
  doc["network"]["mqtt"]["client"]["id"] = network::mqtt::client_id;

  // Wi-Fi configuration
  doc["network"]["wifi"]["ssid"] = network::wifi::ssid;
  doc["network"]["wifi"]["password"] = network::wifi::password;

  // Storage configuration
  // SD confiugration
  doc["storage"]["sd"]["pins"]["mosi"] = storage::sd::pins_mosi;
  doc["storage"]["sd"]["pins"]["miso"] = storage::sd::pins_miso;
  doc["storage"]["sd"]["pins"]["clk"] = storage::sd::pins_clk;
  doc["storage"]["sd"]["pins"]["cs"] = storage::sd::pins_cs;

  // Update configuration
  doc["update"]["url"] = update::ota::manifest_url;
  doc["update"]["interval"] = update::ota::check_interval;

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
