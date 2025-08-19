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

Wifi::Wifi() {
  Serial.println("Wi-Fi: Initializing the Wi-Fi component...");
  // set the Wi-Fi mode
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("esPINDMD");
  // set dark theme
  _wm.setClass("invert");
  // auto close configportal three minutes
  _wm.setConfigPortalTimeout(180);
  // create a Wi-Fi AP
  bool res;
  res = _wm.autoConnect("esPINMDMD", "pinball12345");
  if (!res) {
    Serial.println("Wi-Fi: Failed to connect or hit timeout");
  } else {
    Serial.println("Wi-Fi: Connected to Wi-Fi");
  }
  Serial.println("Wi-Fi: Initialized the Wi-Fi component!");
}
void Wifi::reset() { _wm.resetSettings(); }
