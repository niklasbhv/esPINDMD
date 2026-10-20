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

#include "update/Version.hpp"

namespace network::mqtt {

/**
 * Constructor of the Mqtt class
 *
 * @param handlers Functions used to apply commands and retrieve states
 */
Mqtt::Mqtt(Handlers handlers)
    : _mqtt_client(_wifi_client), _handlers(std::move(handlers)) {}

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
  _light_state_topic = base + "/matrix/state";
  _light_command_topic = base + "/matrix/set";
  _update_state_topic = base + "/update/state";
  _update_command_topic = base + "/update/install";

  _server = server;
  _mqtt_client.setServer(_server.c_str(), port);
  _mqtt_client.setBufferSize(BUFFER_SIZE);
  _mqtt_client.setCallback(
      [this](char* topic, uint8_t* payload, size_t length) {
        onMessage(topic, payload, length);
      });

  connect();
}

/**
 * Function used to connect to the broker. On success the discovery,
 * availability and current states are published and the command topics
 * are subscribed.
 */
bool Mqtt::connect() {
  if (WiFi.status() != WL_CONNECTED) return false;

  Serial.printf("MQTT: Connecting to %s:%u...\r\n", _server.c_str(), port);
  const char* user = username.empty() ? nullptr : username.c_str();
  const char* pass = password.empty() ? nullptr : password.c_str();
  // The last will marks the device as unavailable if the connection drops
  if (!_mqtt_client.connect(_client_id.c_str(), user, pass,
                            _availability_topic.c_str(), 0, true, "offline")) {
    Serial.printf("MQTT: Connection failed, state %d\r\n",
                  _mqtt_client.state());
    return false;
  }
  Serial.println("MQTT: Connected to server!");

  publishDiscovery();
  _mqtt_client.publish(_availability_topic.c_str(), "online", true);
  _mqtt_client.subscribe(_light_command_topic.c_str());
  _mqtt_client.subscribe(_update_command_topic.c_str());
  publishLightState();
  publishUpdateState();
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
  // Without Wi-Fi there is nothing to back off from
  if (WiFi.status() != WL_CONNECTED) return;
  uint32_t now = millis();
  if (now - _last_reconnect_attempt < _reconnect_interval) return;
  _last_reconnect_attempt = now;
  if (connect()) {
    _reconnect_interval = RECONNECT_INTERVAL_MS;
  } else {
    _reconnect_interval =
        std::min(_reconnect_interval * 2, MAX_RECONNECT_INTERVAL_MS);
  }
}

/**
 * Function used to add the device information shared by all entities
 */
void Mqtt::addDevice(JsonDocument& doc) {
  JsonObject device = doc["device"].to<JsonObject>();
  device["identifiers"].add(_node_id);
  device["name"] = "esPINDMD";
  device["model"] = "esPINDMD";
  device["model_id"] = ESPINDMD_ENV;
  device["sw_version"] = ESPINDMD_VERSION;
}

/**
 * Function used to publish the Home Assistant discovery of all entities
 * see https://www.home-assistant.io/integrations/light.mqtt/#json-schema
 * and https://www.home-assistant.io/integrations/update.mqtt/
 */
void Mqtt::publishDiscovery() {
  std::string payload;
  std::string prefix = std::string(DISCOVERY_PREFIX);

  // Matrix as light with on/off and brightness
  JsonDocument light;
  addDevice(light);
  light["name"] = "Matrix";
  light["unique_id"] = _node_id + "_matrix";
  light["schema"] = "json";
  light["availability_topic"] = _availability_topic;
  light["command_topic"] = _light_command_topic;
  light["state_topic"] = _light_state_topic;
  light["supported_color_modes"].add("brightness");
  serializeJson(light, payload);
  // Retained, so Home Assistant picks up the device after a restart
  if (!_mqtt_client.publish(
          (prefix + "/light/" + _node_id + "/matrix/config").c_str(),
          payload.c_str(), true)) {
    Serial.println("MQTT: Failed to publish light discovery!");
  }

  // Firmware as update entity
  JsonDocument update;
  addDevice(update);
  update["name"] = "Firmware";
  update["unique_id"] = _node_id + "_firmware";
  update["device_class"] = "firmware";
  update["entity_category"] = "config";
  update["availability_topic"] = _availability_topic;
  update["state_topic"] = _update_state_topic;
  update["command_topic"] = _update_command_topic;
  update["payload_install"] = PAYLOAD_INSTALL;
  payload.clear();
  serializeJson(update, payload);
  if (!_mqtt_client.publish(
          (prefix + "/update/" + _node_id + "/firmware/config").c_str(),
          payload.c_str(), true)) {
    Serial.println("MQTT: Failed to publish update discovery!");
  }
}

/**
 * Function used to publish the current state of the matrix
 */
void Mqtt::publishLightState() {
  if (!_mqtt_client.connected()) return;

  LightState state = _handlers.get_light_state();
  JsonDocument doc;
  doc["state"] = state.on ? "ON" : "OFF";
  doc["brightness"] = state.brightness;
  doc["color_mode"] = "brightness";

  std::string payload;
  serializeJson(doc, payload);
  _mqtt_client.publish(_light_state_topic.c_str(), payload.c_str(), true);
}

/**
 * Function used to publish the current state of the firmware update
 */
void Mqtt::publishUpdateState() {
  if (!_mqtt_client.connected()) return;

  UpdateState state = _handlers.get_update_state();
  JsonDocument doc;
  doc["installed_version"] = state.installed_version;
  doc["latest_version"] = state.latest_version;
  if (!state.release_url.empty()) doc["release_url"] = state.release_url;
  if (!state.release_summary.empty()) {
    doc["release_summary"] = state.release_summary;
  }
  doc["in_progress"] = state.in_progress;
  if (state.percentage >= 0) {
    doc["update_percentage"] = state.percentage;
  } else {
    doc["update_percentage"] = nullptr;
  }

  std::string payload;
  serializeJson(doc, payload);
  _mqtt_client.publish(_update_state_topic.c_str(), payload.c_str(), true);
}

/**
 * Callback invoked whenever a message gets published to a subscribed topic
 */
void Mqtt::onMessage(char* topic, uint8_t* payload, size_t length) {
  if (_light_command_topic == topic) {
    onLightCommand(payload, length);
  } else if (_update_command_topic == topic) {
    if (length == strlen(PAYLOAD_INSTALL) &&
        memcmp(payload, PAYLOAD_INSTALL, length) == 0) {
      _handlers.on_install();
    }
  }
}

/**
 * Function used to handle light commands.
 * Expects payloads like {"state": "ON", "brightness": 128}
 */
void Mqtt::onLightCommand(uint8_t* payload, size_t length) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    Serial.printf("MQTT: Invalid command: %s\r\n", err.c_str());
    return;
  }

  LightCommand command;
  if (doc["state"].is<const char*>()) {
    command.on = strcmp(doc["state"].as<const char*>(), "ON") == 0;
  }
  if (doc["brightness"].is<uint8_t>()) {
    command.brightness = doc["brightness"].as<uint8_t>();
  }
  _handlers.on_light_command(command);
  // Confirm the new state, Home Assistant waits for it
  publishLightState();
}

/**
 * Function used to check if the client is connected to the broker
 */
bool Mqtt::connected() { return _mqtt_client.connected(); }

}  // namespace network::mqtt
