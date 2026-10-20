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

#include "Wifi.hpp"

namespace network::wifi {

/**
 * Constructor of the Wifi class, connects to the stored network or opens
 * the setup portal if that fails. Blocks until connected or the portal
 * timed out.
 *
 * @param on_portal Invoked once the setup portal is opened
 */
Wifi::Wifi(PortalCallback on_portal) {
  Serial.println("Wi-Fi: Initializing the Wi-Fi component...");
  // set the Wi-Fi mode
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("esPINDMD");
  // set dark theme
  _wm.setClass("invert");
  // auto close the setup portal after the timeout
  _wm.setConfigPortalTimeout(PORTAL_TIMEOUT_S);
  _wm.setAPCallback([this, on_portal](WiFiManager*) {
    _portal_opened = true;
    Serial.printf(
        "Wi-Fi: Opened the setup portal, connect to %s with "
        "password %s and open http://%s\r\n",
        AP_SSID, AP_PASSWORD, WiFi.softAPIP().toString().c_str());
    if (on_portal) on_portal();
  });
  // create a Wi-Fi AP
  bool res;
  res = _wm.autoConnect(AP_SSID, AP_PASSWORD);
  if (!res) {
    Serial.println("Wi-Fi: Failed to connect or hit timeout");
  } else {
    Serial.println("Wi-Fi: Connected to Wi-Fi");
  }
  Serial.println("Wi-Fi: Initialized the Wi-Fi component!");
}
void Wifi::reset() { _wm.resetSettings(); }

bool Wifi::portalOpened() const { return _portal_opened; }

}  // namespace network::wifi
