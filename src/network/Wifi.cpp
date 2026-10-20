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

#include <esp_mac.h>

namespace network::wifi {

Wifi::Wifi(PortalCallbacks callbacks) : _callbacks(std::move(callbacks)) {}

/**
 * Function used to connect to the configured network, opens the setup
 * portal if that fails. Only blocks while trying to connect.
 */
void Wifi::begin() {
  Serial.println("Wi-Fi: Initializing the Wi-Fi component...");
  // The hostname contains the end of the MAC address to tell multiple
  // devices apart, it has to be set before the Wi-Fi is started. It is also
  // used as name of the access point of the setup portal.
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char hostname[20];
  snprintf(hostname, sizeof(hostname), "esPINDMD-%02x%02x%02x", mac[3], mac[4],
           mac[5]);
  _hostname = hostname;
  WiFi.setHostname(_hostname.c_str());
  _wm.setHostname(_hostname.c_str());
  // set the Wi-Fi mode
  WiFi.mode(WIFI_STA);
  Serial.printf("Wi-Fi: Hostname %s\r\n", _hostname.c_str());

  // Generated after starting the Wi-Fi, as the random number generator
  // only produces true random numbers while the radio is running
  _ap_password.clear();
  size_t characters = strlen(AP_PASSWORD_CHARACTERS);
  for (size_t i = 0; i < AP_PASSWORD_LENGTH; i++) {
    _ap_password += AP_PASSWORD_CHARACTERS[esp_random() % characters];
  }
  // set dark theme
  _wm.setClass("invert");
  // run the portal in the background, it is processed by loop()
  _wm.setConfigPortalBlocking(false);
  // auto close the setup portal after the timeout
  _wm.setConfigPortalTimeout(PORTAL_TIMEOUT_S);
  _wm.setAPCallback([this](WiFiManager*) {
    _portal_active = true;
    Serial.printf(
        "Wi-Fi: Opened the setup portal, connect to %s with "
        "password %s and open http://%s\r\n",
        _hostname.c_str(), _ap_password.c_str(),
        WiFi.softAPIP().toString().c_str());
    if (_callbacks.on_open) _callbacks.on_open();
  });
  // Keep the credentials entered in the portal in the configuration,
  // otherwise configured credentials would replace them on the next boot
  _wm.setSaveConfigCallback([this]() {
    ssid = _wm.getWiFiSSID().c_str();
    password = _wm.getWiFiPass().c_str();
    _credentials_saved = true;
  });

  // Configured credentials take precedence over the ones of the portal
  if (!ssid.empty()) {
    Serial.printf("Wi-Fi: Connecting to %s\r\n", ssid.c_str());
    _wm.preloadWiFi(ssid.c_str(), password.c_str());
  }
  if (_wm.autoConnect(_hostname.c_str(), _ap_password.c_str())) {
    Serial.printf("Wi-Fi: Connected to %s\r\n", WiFi.SSID().c_str());
  } else if (!_portal_active) {
    Serial.println("Wi-Fi: Failed to connect");
  }
  Serial.println("Wi-Fi: Initialized the Wi-Fi component!");
}

/**
 * Function used to process the setup portal while it is open
 */
void Wifi::loop() {
  if (!_portal_active) return;
  _wm.process();
  if (_wm.getConfigPortalActive()) return;

  // The portal closes after saving working credentials or the timeout
  _portal_active = false;
  bool connected = _credentials_saved && WiFi.status() == WL_CONNECTED;
  Serial.println(connected ? "Wi-Fi: Connected using the setup portal"
                           : "Wi-Fi: Setup portal timed out");
  if (_callbacks.on_close) _callbacks.on_close(connected);
}

/**
 * Function used to forget all credentials, the setup portal opens on the
 * next boot
 */
void Wifi::reset() {
  ssid.clear();
  password.clear();
  _wm.resetSettings();
}

bool Wifi::portalActive() const { return _portal_active; }

const std::string& Wifi::apSsid() const { return _hostname; }

const std::string& Wifi::apPassword() const { return _ap_password; }

const std::string& Wifi::hostname() const { return _hostname; }

}  // namespace network::wifi
