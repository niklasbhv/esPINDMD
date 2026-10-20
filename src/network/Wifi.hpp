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

inline std::string ssid = "";
inline std::string password = "";

// Access point opened by the setup portal if no network is configured
constexpr const char* AP_SSID = "esPINDMD";
constexpr const char* AP_PASSWORD = "pinball12345";
// Time after which the setup portal closes and the device starts offline
constexpr uint16_t PORTAL_TIMEOUT_S = 180;

// Invoked once the setup portal is opened, e.g. to show the access point
using PortalCallback = std::function<void()>;

class Wifi {
 private:
  WiFiManager _wm;
  bool _portal_opened = false;

 public:
  explicit Wifi(PortalCallback on_portal = nullptr);
  void reset();
  bool portalOpened() const;
};

}  // namespace network::wifi
