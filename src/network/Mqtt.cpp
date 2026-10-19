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

namespace network::mqtt {

/**
 * Constructor of the Mqtt class
 *
 * @param on_command Invoked whenever a command for the matrix is received
 * @param get_state Used to retrieve the current state of the matrix
 */
Mqtt::Mqtt(CommandHandler on_command, StateProvider get_state)
    : _mqtt_client(_wifi_client),
      _on_command(std::move(on_command)),
      _get_state(std::move(get_state)) {}

/**
 * Function used to set up the client and topics, needs to be called
 * after the Wi-Fi component is initialized
 */
void Mqtt::begin() {
  // Home Assistant only allows [a-zA-Z0-9_-] in node ids
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  _node_id = std::string("espindmd_") + mac.c_str();
  _client_id = client_id.empty() ? _node_id : client_id;

  std::string base = "espindmd/" + _node_id;
  _availability_topic = base + "/availability";
  _state_topic = base + "/matrix/state";
  _command_topic = base + "/matrix/set";
  _discovery_topic =
      std::string(DISCOVERY_PREFIX) + "/light/" + _node_id + "/matrix/config";

  _mqtt_client.setServer(server.c_str(), port);
  _mqtt_client.setBufferSize(BUFFER_SIZE);
  _mqtt_client.setCallback([this](char* topic, uint8_t* payload,
                                  size_t length) {
    onMessage(topic, payload, length);
  });

  connect();
}

/**
 * Function used to connect to the broker. On success the discovery,
 * availability and current state are published and the command topic
 * is subscribed.
 */
bool Mqtt::connect() {
  if (WiFi.status() != WL_CONNECTED) return false;

  Serial.printf("MQTT: Connecting to %s:%u...\n", server.c_str(), port);
  const char* user = username.empty() ? nullptr : username.c_str();
  const char* pass = password.empty() ? nullptr : password.c_str();
  // The last will marks the device as unavailable if the connection drops
  if (!_mqtt_client.connect(_client_id.c_str(), user, pass,
                            _availability_topic.c_str(), 0, true,
                            "offline")) {
    Serial.printf("MQTT: Connection failed, state %d\n", _mqtt_client.state());
    return false;
  }
  Serial.println("MQTT: Connected to server!");

  publishDiscovery();
  _mqtt_client.publish(_availability_topic.c_str(), "online", true);
  _mqtt_client.subscribe(_command_topic.c_str());
  publishState();
  return true;
}

/**
 * Function used to keep the connection alive and process incoming
 * messages, reconnects in a non-blocking manner if the connection is lost.
 * Needs to be called frequently.
 */
void Mqtt::loop() {
  if (_mqtt_client.connected()) {
    _mqtt_client.loop();
    return;
  }
  uint32_t now = millis();
  if (now - _last_reconnect_attempt >= RECONNECT_INTERVAL_MS) {
    _last_reconnect_attempt = now;
    connect();
  }
}

/**
 * Function used to publish the matrix as a Home Assistant light,
 * see https://www.home-assistant.io/integrations/light.mqtt/#json-schema
 */
void Mqtt::publishDiscovery() {
  JsonDocument doc;
  // Device information
  JsonObject device = doc["device"].to<JsonObject>();
  device["identifiers"].add(_node_id);
  device["name"] = "esPINDMD";
  device["model"] = "esPINDMD";
  device["sw_version"] = "0.0.0";

  // Matrix capabilities
  doc["name"] = "Matrix";
  doc["unique_id"] = _node_id + "_matrix";
  doc["schema"] = "json";
  doc["availability_topic"] = _availability_topic;
  doc["command_topic"] = _command_topic;
  doc["state_topic"] = _state_topic;
  doc["supported_color_modes"].add("brightness");

  std::string payload;
  serializeJson(doc, payload);
  // Retained, so Home Assistant picks up the device after a restart
  if (!_mqtt_client.publish(_discovery_topic.c_str(), payload.c_str(), true)) {
    Serial.println("MQTT: Failed to publish discovery!");
  }
}

/**
 * Function used to publish the current state of the matrix
 */
void Mqtt::publishState() {
  if (!_mqtt_client.connected()) return;

  LightState state = _get_state();
  JsonDocument doc;
  doc["state"] = state.on ? "ON" : "OFF";
  doc["brightness"] = state.brightness;
  doc["color_mode"] = "brightness";

  std::string payload;
  serializeJson(doc, payload);
  _mqtt_client.publish(_state_topic.c_str(), payload.c_str(), true);
}

/**
 * Callback invoked whenever a message gets published to a subscribed topic.
 * Expects payloads like {"state": "ON", "brightness": 128}
 */
void Mqtt::onMessage(char* topic, uint8_t* payload, size_t length) {
  if (_command_topic != topic) return;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    Serial.printf("MQTT: Invalid command: %s\n", err.c_str());
    return;
  }

  LightCommand command;
  if (doc["state"].is<const char*>()) {
    command.on = strcmp(doc["state"].as<const char*>(), "ON") == 0;
  }
  if (doc["brightness"].is<uint8_t>()) {
    command.brightness = doc["brightness"].as<uint8_t>();
  }
  _on_command(command);
  // Confirm the new state, Home Assistant waits for it
  publishState();
}

/**
 * Function used to check if the client is connected to the broker
 */
bool Mqtt::connected() { return _mqtt_client.connected(); }

}  // namespace network::mqtt
