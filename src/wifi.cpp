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

#include <WiFiManager.h>

WiFiManager wm;

void setupWifi() {
  // set the Wi-Fi mode
  WiFi.mode(WIFI_STA);
  // set dark theme
  wm.setClass("invert");
  // auto close configportal three minutes
  wm.setConfigPortalTimeout(180);
  // create a Wi-Fi AP
  bool res;
  res = wm.autoConnect("ESPINMDMD", "pinball");
  if (!res) {
    Serial.println("Failed to connect or hit timeout");
  } else {
    Serial.println("Connected to Wi-Fi");
  }
}

void clearWifi() {
  wm.resetSettings();
}