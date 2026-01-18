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

void Configuration::load() {}

void Configuration::loadDefault() {
  // Device values
  String deviceId = "esPINDMD-" + WiFi.macAddress();
  strlcpy(deviceName, deviceId.c_str(), sizeof(deviceName));

  // Clock values
  clockColourR = 231;
  clockColourG = 103;
  clockColourB = 3;
  strlcpy(clockNtpServer, "pool.ntp.org", sizeof(clockNtpServer));
  strlcpy(clockNtpTimezone, "Europe/Berlin", sizeof(clockNtpTimezone));
  clockNtpSyncTimeout = 60;

  // Mqtt values
  mqttEnabled = false;
  strlcpy(mqttServer, "", sizeof(mqttServer));
  mqttPort = 1883;
  strlcpy(mqttCredentialsUsername, "", sizeof(mqttCredentialsUsername));
  strlcpy(mqttCredentialsPassword, "", sizeof(mqttCredentialsPassword));
}

void Configuration::save() {
  FsFile configFile;
  Sd::openFile(CONFIG_FILE_PATH, configFile);
  JsonDocument doc;

  // Device values
  doc["device"]["name"] = deviceName;

  // Clock values
  doc["clock"]["colour"]["r"] = clockColourR;
  doc["clock"]["colour"]["g"] = clockColourG;
  doc["clock"]["colour"]["b"] = clockColourB;
  doc["clock"]["ntp"]["server"] = clockNtpServer;
  doc["clock"]["ntp"]["timezone"] = clockNtpTimezone;
  doc["clock"]["ntp"]["sync_timeout"] = clockNtpSyncTimeout;

  // Mqtt values
  doc["mqtt"]["enabled"] = mqttEnabled;
  doc["mqtt"]["server"] = mqttServer;
  doc["mqtt"]["port"] = mqttPort;
  doc["mqtt"]["credentials"]["username"] = mqttCredentialsUsername;
  doc["mqtt"]["credentials"]["password"] = mqttCredentialsPassword;

  if (serializeJsonPretty(doc, configFile) == 0) {
    Serial.println("Configuration: Failed to write configuration to SD");
  }
  configFile.close();
}
