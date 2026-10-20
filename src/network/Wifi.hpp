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

#include <WiFiManager.h>

#include <functional>
#include <string>

namespace network::wifi {

// Network to connect to, empty to use the network set up in the portal
inline std::string ssid = "";
inline std::string password = "";

// The access point opened by the setup portal if no network is configured
// uses the hostname as name, e.g. esPINDMD-a1b2c3.
// The password of the access point is generated randomly on every boot,
// ambiguous characters like l, 1, o and 0 are left out
constexpr const char* AP_PASSWORD_CHARACTERS =
    "abcdefghijkmnpqrstuvwxyz23456789";
constexpr size_t AP_PASSWORD_LENGTH = 10;
// Time after which the setup portal closes and the device continues offline
constexpr uint16_t PORTAL_TIMEOUT_S = 180;

/**
 * Functions invoked when the setup portal is opened and closed
 */
struct PortalCallbacks {
  std::function<void()> on_open;
  // Invoked with true if new credentials were saved and connected
  std::function<void(bool connected)> on_close;
};

/**
 * The Wi-Fi component connects to the configured network. If that fails,
 * a setup portal is opened, which runs in the background so the rest of
 * the device, e.g. the CLI, keeps working.
 */
class Wifi {
 private:
  WiFiManager _wm;
  PortalCallbacks _callbacks;
  bool _portal_active = false;
  bool _credentials_saved = false;
  std::string _hostname;
  std::string _ap_password;

 public:
  explicit Wifi(PortalCallbacks callbacks);
  void begin();
  void loop();
  void reset();
  bool portalActive() const;
  const std::string& apSsid() const;
  const std::string& apPassword() const;
  const std::string& hostname() const;
};

}  // namespace network::wifi
