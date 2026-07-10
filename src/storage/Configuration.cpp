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

Configuration::Configuration() {}

void Configuration::load() {
  FsFile config_file;
  Sd::openFile(CONFIG_FILE_PATH, config_file);
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, config_file);
  Sd::closeFile(config_file);

  if (err) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(err.c_str());
    return;
  }

  // Device values
  strlcpy(deviceName, doc["device"]["name"], sizeof(deviceName));

  // Clock values
  application::clock::colour_r = doc["application"]["clock"]["colour"]["r"];
  application::clock::colour_g = doc["application"]["clock"]["colour"]["g"];
  application::clock::colour_b = doc["application"]["clock"]["colour"]["b"];
  application::clock::ntp_server =
      doc["application"]["clock"]["ntp"]["server"].as<std::string>();
  application::clock::ntp_timezone =
      doc["application"]["clock"]["ntp"]["timezone"].as<std::string>();
  application::clock::ntp_sync_timeout =
      doc["application"]["clock"]["ntp"]["sync"]["timeout"];

  // Mqtt values
  network::mqtt::enable = doc["network"]["mqtt"]["enable"];
  network::mqtt::server = doc["network"]["mqtt"]["server"].as<std::string>();
  network::mqtt::port = doc["network"]["mqtt"]["port"];
  network::mqtt::username =
      doc["network"]["mqtt"]["credentials"]["username"].as<std::string>();
  network::mqtt::password =
      doc["network"]["mqtt"]["credentials"]["password"].as<std::string>();
}

void Configuration::loadDefault() {
  // Device values
  String deviceId = "esPINDMD-" + WiFi.macAddress();
  strlcpy(deviceName, deviceId.c_str(), sizeof(deviceName));
}

void Configuration::save() {
  FsFile config_file;
  Sd::openFile(CONFIG_FILE_PATH, config_file);
  JsonDocument doc;

  // Device values
  doc["device"]["name"] = deviceName;

  // Application configuration
  // Clock configuration
  doc["application"]["clock"]["colour"]["r"] = application::clock::colour_r;
  doc["application"]["clock"]["colour"]["g"] = application::clock::colour_r;
  doc["application"]["clock"]["colour"]["b"] = application::clock::colour_r;
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

  if (serializeJsonPretty(doc, config_file) == 0) {
    Serial.println("Configuration: Failed to write configuration to SD");
  }
  config_file.close();
}
