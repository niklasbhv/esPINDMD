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

inline boolean enable = false;
inline std::string server = "mqtt.example.org";
inline uint16_t port = 1883;
inline std::string username = "";
inline std::string password = "";
inline std::string client_id = "";

// Home Assistant discovery prefix
constexpr const char* DISCOVERY_PREFIX = "homeassistant";
// Minimum time between two reconnection attempts
constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;
// Discovery payloads exceed the default PubSubClient buffer of 256 bytes
constexpr size_t BUFFER_SIZE = 1024;

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

using CommandHandler = std::function<void(const LightCommand&)>;
using StateProvider = std::function<LightState()>;

/**
 * MQTT component exposing the matrix as a Home Assistant light
 * (JSON schema) with on/off and brightness support.
 */
class Mqtt {
 private:
  WiFiClient _wifi_client;
  PubSubClient _mqtt_client;
  CommandHandler _on_command;
  StateProvider _get_state;
  uint32_t _last_reconnect_attempt = 0;

  std::string _client_id;
  std::string _node_id;
  std::string _availability_topic;
  std::string _state_topic;
  std::string _command_topic;
  std::string _discovery_topic;

  bool connect();
  void publishDiscovery();
  void onMessage(char* topic, uint8_t* payload, size_t length);

 public:
  Mqtt(CommandHandler on_command, StateProvider get_state);
  void begin();
  void loop();
  void publishState();
  bool connected();
};

}  // namespace network::mqtt
