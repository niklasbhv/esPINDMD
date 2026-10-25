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

#include <Arduino.h>
#include <HTTPClient.h>

#include <functional>
#include <string>

#include "Version.hpp"

namespace update::ota {

// URL of the release manifest, either the latest GitHub release or a
// manifest.json on a local webserver
inline std::string manifest_url =
    "https://github.com/niklasbhv/esPINDMD/releases/latest/download/"
    "manifest.json";
// Hours between two checks for a new release, 0 only checks on boot
inline uint16_t check_interval = 6;
// Install new releases automatically once they are found
inline bool auto_install = false;

// Time a new firmware has to prove itself before it gets rolled back
constexpr uint32_t CONFIRM_TIMEOUT_MS = 5 * 60 * 1000;
// Time without receiving data before a download is aborted
constexpr uint32_t DOWNLOAD_TIMEOUT_MS = 15000;
// Location of the last installed version in the internal flash (NVS), used
// to detect a release that was rolled back
constexpr const char* NVS_NAMESPACE = "espindmd_ota";
constexpr const char* NVS_KEY_INSTALLED = "installed";

/**
 * Release information from the manifest for the running board
 */
struct Release {
  std::string version;
  std::string release_url;
  std::string release_summary;
  std::string url;
  std::string sha256;
  size_t size = 0;
};

using ProgressCallback = std::function<void(uint8_t percent)>;

/**
 * The Ota class checks a manifest for new releases and installs the
 * firmware for the running board into the inactive OTA partition.
 *
 * Manifest format, URLs may be absolute or relative to the manifest:
 * {
 *   "version": "0.1.0",
 *   "release_url": "https://github.com/...",
 *   "release_summary": "...",
 *   "builds": {
 *     "esp32dev": {"url": "esPINDMD-esp32dev.bin", "size": 1234,
 *                  "sha256": "..."}
 *   }
 * }
 *
 * A newly installed firmware is only marked as valid after confirm() is
 * called, otherwise the bootloader rolls back to the previous firmware.
 */
class Ota {
 private:
  Release _latest;
  bool _checked = false;
  bool _attempted = false;
  bool _install_requested = false;
  bool _installing = false;
  int8_t _progress = -1;
  uint32_t _last_check = 0;

  bool get(const std::string& url,
           const std::function<bool(HTTPClient&)>& handler);
  std::string resolve(const std::string& url) const;
  bool download(HTTPClient& http, const ProgressCallback& progress);
  static std::string lastInstalled();
  static void setLastInstalled(const std::string& version);
  bool rolledBack() const;

 public:
  void begin();
  bool loop();
  bool check();
  bool install(const ProgressCallback& progress);

  bool updateAvailable() const;
  void requestInstall();
  bool installRequested() const;
  bool installing() const;
  int8_t progress() const;
  const Release& latest() const;
  bool checked() const;

  static const char* installedVersion();
  static bool pendingConfirm();
  static void confirm();
};

}  // namespace update::ota
