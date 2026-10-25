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

#include "Settings.hpp"

#include <cstring>

#include "application/Clock.hpp"
#include "display/Matrix.hpp"
#include "network/Mqtt.hpp"
#include "network/Wifi.hpp"
#include "storage/Sd.hpp"
#include "sensor/LightSensor.hpp"
#include "update/Ota.hpp"

namespace storage::settings {

namespace clock = application::clock;
namespace matrix = display::matrix;
namespace mqtt = network::mqtt;
namespace wifi = network::wifi;
namespace sd = storage::sd;
namespace light = sensor::light;
namespace ota = update::ota;

// GPIO range covering the ESP32 and the ESP32-S3, -1 marks unused pins
constexpr int32_t PIN_MIN = -1;
constexpr int32_t PIN_MAX = 48;

// clang-format off
Setting all[] = {
  // Application configuration
  {"application/clock/colour/r", &clock::colour_r, 0, 255, NONE, "Red component of the clock colour"},
  {"application/clock/colour/g", &clock::colour_g, 0, 255, NONE, "Green component of the clock colour"},
  {"application/clock/colour/b", &clock::colour_b, 0, 255, NONE, "Blue component of the clock colour"},
  {"application/clock/ntp/server", &clock::ntp_server, 0, 63, REBOOT, "NTP server used to sync the time"},
  {"application/clock/ntp/timezone", &clock::ntp_timezone, 0, 63, NONE, "POSIX timezone, e.g. CET-1CEST,M3.5.0,M10.5.0/3"},
  {"application/clock/ntp/interval", &clock::ntp_interval, 1, 1440, REBOOT, "Minutes between two NTP syncs"},
  {"application/gif/random", &sd::play_random, 0, 1, NONE, "Play the GIFs in random order"},

  // Display configuration
  {"display/matrix/panel/resolution/x", &matrix::panel_res_x, 1, 255, REBOOT, "Horizontal pixels per panel"},
  {"display/matrix/panel/resolution/y", &matrix::panel_res_y, 1, 255, REBOOT, "Vertical pixels per panel"},
  {"display/matrix/panel/count/x", &matrix::panel_count_x, 1, 8, REBOOT, "Number of panels on the x axis"},
  {"display/matrix/panel/count/y", &matrix::panel_count_y, 1, 1, REBOOT, "Number of panels on the y axis, only 1 is supported"},
  {"display/matrix/pins/r1", &matrix::pins_r1, PIN_MIN, PIN_MAX, REBOOT, "HUB75 R1 pin"},
  {"display/matrix/pins/g1", &matrix::pins_g1, PIN_MIN, PIN_MAX, REBOOT, "HUB75 G1 pin"},
  {"display/matrix/pins/b1", &matrix::pins_b1, PIN_MIN, PIN_MAX, REBOOT, "HUB75 B1 pin"},
  {"display/matrix/pins/r2", &matrix::pins_r2, PIN_MIN, PIN_MAX, REBOOT, "HUB75 R2 pin"},
  {"display/matrix/pins/g2", &matrix::pins_g2, PIN_MIN, PIN_MAX, REBOOT, "HUB75 G2 pin"},
  {"display/matrix/pins/b2", &matrix::pins_b2, PIN_MIN, PIN_MAX, REBOOT, "HUB75 B2 pin"},
  {"display/matrix/pins/a", &matrix::pins_a, PIN_MIN, PIN_MAX, REBOOT, "HUB75 A pin"},
  {"display/matrix/pins/b", &matrix::pins_b, PIN_MIN, PIN_MAX, REBOOT, "HUB75 B pin"},
  {"display/matrix/pins/c", &matrix::pins_c, PIN_MIN, PIN_MAX, REBOOT, "HUB75 C pin"},
  {"display/matrix/pins/d", &matrix::pins_d, PIN_MIN, PIN_MAX, REBOOT, "HUB75 D pin"},
  {"display/matrix/pins/e", &matrix::pins_e, PIN_MIN, PIN_MAX, REBOOT, "HUB75 E pin, -1 for panels without it"},
  {"display/matrix/pins/lat", &matrix::pins_lat, PIN_MIN, PIN_MAX, REBOOT, "HUB75 LAT pin"},
  {"display/matrix/pins/oe", &matrix::pins_oe, PIN_MIN, PIN_MAX, REBOOT, "HUB75 OE pin"},
  {"display/matrix/pins/clk", &matrix::pins_clk, PIN_MIN, PIN_MAX, REBOOT, "HUB75 CLK pin"},
  {"display/matrix/brightness", &matrix::brightness, 0, 255, NONE, "Brightness of the matrix"},
  {"display/matrix/dimming/min", &matrix::dimming_min, 0, 255, NONE, "Brightness in the dark when a light sensor is connected"},

  // Network configuration
  {"network/mqtt/enable", &mqtt::enable, 0, 1, REBOOT, "Connect to a MQTT broker"},
  {"network/mqtt/server", &mqtt::server, 0, 63, REBOOT, "Hostname or IP of the MQTT broker"},
  {"network/mqtt/port", &mqtt::port, 1, 65535, REBOOT, "Port of the MQTT broker"},
  {"network/mqtt/credentials/username", &mqtt::username, 0, 63, REBOOT, "MQTT username, empty for none"},
  {"network/mqtt/credentials/password", &mqtt::password, 0, 63, REBOOT | SECRET, "MQTT password, empty for none"},
  {"network/mqtt/client/id", &mqtt::client_id, 0, 63, REBOOT, "MQTT client id, empty to derive it from the MAC"},
  {"network/wifi/ssid", &wifi::ssid, 0, 32, REBOOT, "Wi-Fi network, empty to use the setup portal"},
  {"network/wifi/password", &wifi::password, 0, 63, REBOOT | SECRET, "Wi-Fi password"},

  // Sensor configuration
  {"sensor/light/enable", &light::enable, 0, 1, REBOOT, "Dim the matrix using a BH1750 light sensor, if connected"},
  {"sensor/light/pins/sda", &light::pins_sda, 0, PIN_MAX, REBOOT, "I2C SDA pin of the light sensor"},
  {"sensor/light/pins/scl", &light::pins_scl, 0, PIN_MAX, REBOOT, "I2C SCL pin of the light sensor"},
  {"sensor/light/lux", &light::full_lux, 1, 65535, NONE, "Ambient light in lux at which the full brightness is used"},

  // Storage configuration
  {"storage/sd/pins/mosi", &sd::pins_mosi, 0, PIN_MAX, REBOOT, "SD card MOSI pin"},
  {"storage/sd/pins/miso", &sd::pins_miso, 0, PIN_MAX, REBOOT, "SD card MISO pin"},
  {"storage/sd/pins/clk", &sd::pins_clk, 0, PIN_MAX, REBOOT, "SD card CLK pin"},
  {"storage/sd/pins/cs", &sd::pins_cs, 0, PIN_MAX, REBOOT, "SD card CS pin"},

  // Update configuration
  {"update/url", &ota::manifest_url, 0, 255, NONE, "URL of the release manifest"},
  {"update/interval", &ota::check_interval, 0, 8760, NONE, "Hours between update checks, 0 only on boot"},
  {"update/auto", &ota::auto_install, 0, 1, NONE, "Install new firmware automatically"},
};
// clang-format on

const size_t count = sizeof(all) / sizeof(all[0]);

const char* Setting::name() const {
  const char* last = strrchr(path, '/');
  return last ? last + 1 : path;
}

bool Setting::hasFlag(Flags flag) const { return (flags & flag) != 0; }

std::string Setting::toString() const {
  if (const auto* text = std::get_if<std::string*>(&value)) {
    if ((*text)->empty()) return "(empty)";
    if (hasFlag(SECRET)) return "********";
    return **text;
  }
  if (const auto* flag = std::get_if<bool*>(&value)) {
    return **flag ? "on" : "off";
  }
  return std::visit(
      [](auto* number) -> std::string {
        if constexpr (std::is_same_v<decltype(number), std::string*> ||
                      std::is_same_v<decltype(number), bool*>) {
          return "";
        } else {
          return std::to_string(static_cast<int32_t>(*number));
        }
      },
      value);
}

bool Setting::accepts(int32_t number) const {
  if (std::holds_alternative<bool*>(value)) return true;
  // Strings are only limited in length, 0 means unlimited
  if (std::holds_alternative<std::string*>(value)) {
    return max == 0 || number <= max;
  }
  return number >= min && number <= max;
}

std::string Setting::range() const {
  if (std::holds_alternative<bool*>(value)) return "on/off";
  if (std::holds_alternative<std::string*>(value)) {
    return max > 0 ? "max " + std::to_string(max) + " chars, - to clear"
                   : "- to clear";
  }
  return std::to_string(min) + "-" + std::to_string(max);
}

bool Setting::parse(const std::string& input, std::string& error) {
  if (auto* text = std::get_if<std::string*>(&value)) {
    std::string result = input == "-" ? "" : input;
    if (!accepts(result.length())) {
      error = "Too long, at most " + std::to_string(max) + " characters";
      return false;
    }
    **text = result;
    return true;
  }

  if (auto* flag = std::get_if<bool*>(&value)) {
    String lower = String(input.c_str());
    lower.toLowerCase();
    if (lower == "on" || lower == "true" || lower == "yes" || lower == "1") {
      **flag = true;
    } else if (lower == "off" || lower == "false" || lower == "no" ||
               lower == "0") {
      **flag = false;
    } else {
      error = "Expected on or off";
      return false;
    }
    return true;
  }

  char* end = nullptr;
  long number = strtol(input.c_str(), &end, 10);
  if (input.empty() || *end != '\0') {
    error = "Expected a number";
    return false;
  }
  if (!accepts(number)) {
    error = "Out of range, allowed is " + range();
    return false;
  }
  std::visit(
      [number](auto* target) {
        using T = std::remove_pointer_t<decltype(target)>;
        if constexpr (!std::is_same_v<T, std::string> &&
                      !std::is_same_v<T, bool>) {
          *target = static_cast<T>(number);
        }
      },
      value);
  return true;
}

}  // namespace storage::settings
