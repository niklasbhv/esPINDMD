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

#include "Mqtt.hpp"

Mqtt::Mqtt(const char* mqtt_server, uint16_t mqtt_port) {
  PubSubClient _mqttClient(_wifiClient);
  _mqttClient.setServer(mqtt_server, mqtt_port);
  _clientId = "esPINDMD-" + WiFi.macAddress();
}

void Mqtt::callback(char* topic, uint8_t* payload, size_t length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
}

void Mqtt::connect() { _mqttClient.connect(_clientId.c_str()); }

/**
 * Non-blocking function for reconnecting to the MQTT broker
 */
int Mqtt::reconnect() {
  if (_mqttClient.connect(_clientId.c_str())) {
  }
  // return _mqttClient.connected();
  return 0;
}

boolean Mqtt::connected() { return _mqttClient.connected(); }