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

#include "Ota.hpp"

#include <ArduinoJson.h>
#include <Preferences.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_ota_ops.h>
#include <mbedtls/sha256.h>

#include <memory>

// Root certificate bundle shipped with the framework, used to verify HTTPS
// connections, e.g. to GitHub
extern const uint8_t x509_crt_bundle_start[] asm(
    "_binary_x509_crt_bundle_start");
extern const uint8_t x509_crt_bundle_end[] asm("_binary_x509_crt_bundle_end");

/**
 * Overrides the weak function of the Arduino core, which would otherwise mark
 * a new firmware as valid right on boot. The firmware is confirmed by
 * Ota::confirm() once it is up and running.
 */
extern "C" bool verifyRollbackLater() { return true; }

namespace update::ota {

// Time between retries if checking for a new release failed
constexpr uint32_t RETRY_INTERVAL_MS = 10 * 60 * 1000;

void Ota::begin() {
  Serial.printf("OTA: Running firmware %s for %s\r\n", ESPINDMD_VERSION,
                ESPINDMD_ENV);
  if (pendingConfirm()) {
    Serial.println(
        "OTA: Firmware is not confirmed yet, waiting for it to "
        "connect before confirming");
  }
}

/**
 * Function used to periodically check for new releases and to roll back a
 * firmware that did not get confirmed in time. Needs to be called frequently.
 *
 * @return true if the release information changed
 */
bool Ota::loop() {
  if (pendingConfirm() && millis() > CONFIRM_TIMEOUT_MS) {
    Serial.println("OTA: Firmware was not confirmed in time, rolling back!");
    esp_ota_mark_app_invalid_rollback_and_reboot();
  }

  if (WiFi.status() != WL_CONNECTED) return false;
  if (_attempted) {
    // Only check once per boot if periodic checks are disabled
    if (_checked && check_interval == 0) return false;
    uint32_t delay_ms =
        _checked ? check_interval * 60UL * 60UL * 1000UL : RETRY_INTERVAL_MS;
    if (millis() - _last_check < delay_ms) return false;
  }

  _attempted = true;
  _last_check = millis();
  bool ok = check();
  if (ok && auto_install && updateAvailable()) {
    if (rolledBack()) {
      Serial.printf(
          "OTA: Not installing %s automatically, it was rolled "
          "back before\r\n",
          _latest.version.c_str());
    } else {
      Serial.printf("OTA: Installing %s automatically\r\n",
                    _latest.version.c_str());
      requestInstall();
    }
  }
  return ok;
}

/**
 * Function used to download the manifest and read the release for the
 * running board
 *
 * @return true if the manifest was read successfully
 */
bool Ota::check() {
  Serial.printf("OTA: Checking %s\r\n", manifest_url.c_str());
  bool ok = get(manifest_url, [this](HTTPClient& http) {
    // Only keep the build of the running board to save memory
    JsonDocument filter;
    filter["version"] = true;
    filter["release_url"] = true;
    filter["release_summary"] = true;
    filter["builds"][ESPINDMD_ENV] = true;

    JsonDocument doc;
    DeserializationError err = deserializeJson(
        doc, http.getString(), DeserializationOption::Filter(filter));
    if (err) {
      Serial.printf("OTA: Invalid manifest: %s\r\n", err.c_str());
      return false;
    }

    JsonVariantConst build = doc["builds"][ESPINDMD_ENV];
    if (!doc["version"].is<const char*>() || !build["url"].is<const char*>() ||
        !build["sha256"].is<const char*>()) {
      Serial.printf("OTA: Manifest contains no build for %s\r\n", ESPINDMD_ENV);
      return false;
    }

    Release release;
    release.version = doc["version"].as<const char*>();
    release.release_url = doc["release_url"] | "";
    release.release_summary = doc["release_summary"] | "";
    release.url = resolve(build["url"].as<const char*>());
    release.sha256 = build["sha256"].as<const char*>();
    release.size = build["size"] | 0;
    _latest = release;
    return true;
  });

  if (ok) {
    _checked = true;
    Serial.printf("OTA: Latest version %s, installed %s\r\n",
                  _latest.version.c_str(), ESPINDMD_VERSION);
  }
  return ok;
}

/**
 * Function used to download and install the latest release. Blocks until
 * the download is finished, the device needs to be restarted afterwards.
 *
 * @param progress Invoked whenever the progress in percent changes
 * @return true if the new firmware was installed
 */
bool Ota::install(const ProgressCallback& progress) {
  _install_requested = false;
  if (!updateAvailable()) {
    Serial.println("OTA: No update available");
    return false;
  }

  Serial.printf("OTA: Installing %s from %s (free heap: %lu bytes)\r\n",
                _latest.version.c_str(), _latest.url.c_str(),
                static_cast<unsigned long>(ESP.getFreeHeap()));
  _installing = true;
  _progress = 0;
  bool ok = get(_latest.url, [this, &progress](HTTPClient& http) {
    return download(http, progress);
  });
  _installing = false;
  _progress = -1;

  if (ok) {
    // If the device later runs a different version, this one was rolled back
    setLastInstalled(_latest.version);
    Serial.println("OTA: Installed the new firmware!");
  } else {
    Serial.println("OTA: Installation failed!");
  }
  return ok;
}

/**
 * Function used to read the version of the last installed firmware
 */
std::string Ota::lastInstalled() {
  Preferences preferences;
  // Opening read only fails if nothing was stored yet
  if (!preferences.begin(NVS_NAMESPACE, true)) return std::string();
  String version = preferences.isKey(NVS_KEY_INSTALLED)
                       ? preferences.getString(NVS_KEY_INSTALLED)
                       : String();
  preferences.end();
  return version.c_str();
}

void Ota::setLastInstalled(const std::string& version) {
  Preferences preferences;
  if (!preferences.begin(NVS_NAMESPACE, false)) return;
  preferences.putString(NVS_KEY_INSTALLED, version.c_str());
  preferences.end();
}

/**
 * Function used to check if the latest release was installed before but is
 * not running, which means it was rolled back. It is not installed
 * automatically again, as it would most likely be rolled back again.
 */
bool Ota::rolledBack() const {
  return _latest.version != ESPINDMD_VERSION &&
         _latest.version == lastInstalled();
}

/**
 * Function used to write the response body into the inactive OTA partition,
 * the firmware is only activated if the SHA256 matches the manifest
 */
bool Ota::download(HTTPClient& http, const ProgressCallback& progress) {
  int length = http.getSize();
  if (length <= 0) {
    Serial.println("OTA: Server did not send the firmware size");
    return false;
  }
  if (_latest.size != 0 && static_cast<size_t>(length) != _latest.size) {
    Serial.printf("OTA: Size mismatch, expected %u got %d bytes\r\n",
                  _latest.size, length);
    return false;
  }
  if (!Update.begin(length, U_FLASH)) {
    Serial.printf("OTA: %s\r\n", Update.errorString());
    return false;
  }

  mbedtls_sha256_context sha;
  mbedtls_sha256_init(&sha);
  mbedtls_sha256_starts(&sha, 0);

  auto* stream = http.getStreamPtr();
  uint8_t buffer[1024];
  size_t written = 0;
  uint32_t last_data = millis();
  while (written < static_cast<size_t>(length)) {
    size_t available = stream->available();
    if (available == 0) {
      if (millis() - last_data > DOWNLOAD_TIMEOUT_MS) {
        Serial.println("OTA: Download timed out");
        break;
      }
      delay(1);
      continue;
    }

    size_t chunk = std::min(
        {available, sizeof(buffer), static_cast<size_t>(length) - written});
    size_t read = stream->readBytes(buffer, chunk);
    mbedtls_sha256_update(&sha, buffer, read);
    if (Update.write(buffer, read) != read) {
      Serial.printf("OTA: %s\r\n", Update.errorString());
      break;
    }
    written += read;
    last_data = millis();

    int8_t percent = written * 100 / length;
    if (percent != _progress) {
      _progress = percent;
      if (percent % 10 == 0) Serial.printf("OTA: %d%%\r\n", percent);
      if (progress) progress(percent);
    }
  }

  uint8_t digest[32];
  mbedtls_sha256_finish(&sha, digest);
  mbedtls_sha256_free(&sha);

  if (written != static_cast<size_t>(length)) {
    Update.abort();
    return false;
  }

  char hex[sizeof(digest) * 2 + 1];
  for (size_t i = 0; i < sizeof(digest); i++) {
    sprintf(hex + i * 2, "%02x", digest[i]);
  }
  if (strcasecmp(hex, _latest.sha256.c_str()) != 0) {
    Serial.printf("OTA: SHA256 mismatch, expected %s got %s\r\n",
                  _latest.sha256.c_str(), hex);
    Update.abort();
    return false;
  }

  // Validates the image and sets it as the boot partition
  if (!Update.end()) {
    Serial.printf("OTA: %s\r\n", Update.errorString());
    return false;
  }
  return true;
}

/**
 * Function used to send a GET request, uses TLS for https URLs.
 * Redirects are followed, as GitHub redirects release downloads.
 *
 * @param handler Invoked with the response if the request succeeded
 */
bool Ota::get(const std::string& url,
              const std::function<bool(HTTPClient&)>& handler) {
  std::unique_ptr<WiFiClient> client;
  if (url.rfind("https://", 0) == 0) {
    auto secure = std::make_unique<WiFiClientSecure>();
    secure->setCACertBundle(x509_crt_bundle_start,
                            x509_crt_bundle_end - x509_crt_bundle_start);
    client = std::move(secure);
  } else {
    client = std::make_unique<WiFiClient>();
  }

  HTTPClient http;
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.setTimeout(DOWNLOAD_TIMEOUT_MS);
  http.setUserAgent("esPINDMD/" ESPINDMD_VERSION);
  if (!http.begin(*client, url.c_str())) {
    Serial.printf("OTA: Invalid URL %s\r\n", url.c_str());
    return false;
  }

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("OTA: Request to %s failed: %d %s\r\n", url.c_str(), code,
                  code < 0 ? http.errorToString(code).c_str() : "");
    http.end();
    return false;
  }

  bool ok = handler(http);
  http.end();
  return ok;
}

/**
 * Function used to resolve URLs relative to the manifest, so the release
 * files can be served from any webserver without changing the manifest
 */
std::string Ota::resolve(const std::string& url) const {
  if (url.find("://") != std::string::npos) return url;

  if (!url.empty() && url[0] == '/') {
    size_t scheme_end = manifest_url.find("://");
    size_t host_end = manifest_url.find(
        '/', scheme_end == std::string::npos ? 0 : scheme_end + 3);
    return manifest_url.substr(0, host_end) + url;
  }
  return manifest_url.substr(0, manifest_url.rfind('/') + 1) + url;
}

bool Ota::updateAvailable() const {
  return _checked && _latest.version != ESPINDMD_VERSION;
}

/**
 * Function used to request an installation, which is executed by the
 * SystemController outside of the MQTT callback
 */
void Ota::requestInstall() {
  if (updateAvailable()) {
    _install_requested = true;
  } else {
    Serial.println("OTA: Ignoring install request, no update available");
  }
}

bool Ota::installRequested() const { return _install_requested; }

bool Ota::installing() const { return _installing; }

int8_t Ota::progress() const { return _progress; }

const Release& Ota::latest() const { return _latest; }

bool Ota::checked() const { return _checked; }

const char* Ota::installedVersion() { return ESPINDMD_VERSION; }

/**
 * Function used to check if the running firmware still needs to be confirmed
 */
// Cached confirmation state of the running firmware, -1 if not read yet.
// Reading it from the flash on every loop would be wasteful.
static int8_t pending = -1;

bool Ota::pendingConfirm() {
  if (pending < 0) {
    esp_ota_img_states_t state;
    pending = esp_ota_get_state_partition(esp_ota_get_running_partition(),
                                          &state) == ESP_OK &&
              state == ESP_OTA_IMG_PENDING_VERIFY;
  }
  return pending == 1;
}

/**
 * Function used to mark the running firmware as valid, which cancels the
 * rollback to the previous firmware
 */
void Ota::confirm() {
  if (!pendingConfirm()) return;
  if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
    pending = 0;
    Serial.println("OTA: Confirmed the running firmware");
  }
}

}  // namespace update::ota
