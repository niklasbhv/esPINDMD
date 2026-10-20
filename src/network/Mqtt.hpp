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

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include <functional>
#include <optional>
#include <string>

namespace network::mqtt {

inline bool enable = false;
inline std::string server = "mqtt.example.org";
inline uint16_t port = 1883;
inline std::string username = "";
inline std::string password = "";
inline std::string client_id = "";

// Home Assistant discovery prefix
constexpr const char* DISCOVERY_PREFIX = "homeassistant";
// Minimum time between two reconnection attempts
constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;
// The interval doubles after every failed attempt up to this maximum, as
// every attempt blocks while the broker is unreachable
constexpr uint32_t MAX_RECONNECT_INTERVAL_MS = 5 * 60 * 1000;
// Discovery payloads exceed the default PubSubClient buffer of 256 bytes
constexpr size_t BUFFER_SIZE = 1024;
// Payload Home Assistant sends to start an update
constexpr const char* PAYLOAD_INSTALL = "install";

/**
 * State of the matrix as exposed to Home Assistant
 */
struct LightState {
  bool on;
  uint8_t brightness;
};

/**
 * Command received from Home Assistant, fields that were not part of
 * the command are left empty
 */
struct LightCommand {
  std::optional<bool> on;
  std::optional<uint8_t> brightness;
};

/**
 * State of the firmware update as exposed to Home Assistant
 */
struct UpdateState {
  std::string installed_version;
  std::string latest_version;
  std::string release_url;
  std::string release_summary;
  bool in_progress;
  // Progress in percent, negative if no update is in progress
  int8_t percentage;
};

/**
 * Functions used to connect the MQTT component to the rest of the system
 */
struct Handlers {
  std::function<void(const LightCommand&)> on_light_command;
  std::function<LightState()> get_light_state;
  std::function<void()> on_install;
  std::function<UpdateState()> get_update_state;
};

/**
 * MQTT component exposing the matrix as a Home Assistant light
 * (JSON schema) with on/off and brightness support and the firmware
 * as a Home Assistant update entity.
 */
class Mqtt {
 private:
  WiFiClient _wifi_client;
  PubSubClient _mqtt_client;
  Handlers _handlers;
  uint32_t _last_reconnect_attempt = 0;
  uint32_t _reconnect_interval = RECONNECT_INTERVAL_MS;
  // Copy of the server, PubSubClient only keeps a pointer to it
  std::string _server;

  std::string _client_id;
  std::string _node_id;
  std::string _availability_topic;
  std::string _light_state_topic;
  std::string _light_command_topic;
  std::string _update_state_topic;
  std::string _update_command_topic;

  bool connect();
  void addDevice(JsonDocument& doc);
  void publishDiscovery();
  void onMessage(char* topic, uint8_t* payload, size_t length);
  void onLightCommand(uint8_t* payload, size_t length);

 public:
  explicit Mqtt(Handlers handlers);
  void begin();
  void loop();
  void publishLightState();
  void publishUpdateState();
  bool connected();
};

}  // namespace network::mqtt
