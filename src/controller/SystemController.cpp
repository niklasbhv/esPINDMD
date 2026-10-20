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

#include "SystemController.hpp"

/**
 * Function used to initialize the SystemController
 */
void SystemController::begin() {
  // The configuration is loaded from the internal flash first, as it
  // contains the pins of the SD card and the matrix
  storage::configuration::Configuration::load();

  Serial.println("SystemController: Initializing the hardware components...");
  // A config.json on the SD card overrides the stored configuration
  _sd = std::make_unique<storage::sd::Sd>();
  if (_sd->begin()) storage::configuration::Configuration::importFromSd();
  delay(2000);
  // setup the matrix configuration
  display::matrix::Matrix::begin();
  display::matrix::Matrix::printLogo();
  // setup the Wi-Fi configuration, the setup portal runs in the background
  network::wifi::PortalCallbacks callbacks;
  callbacks.on_open = []() {
    // Show how to reach the setup portal while it is open
    display::matrix::Matrix::printLines(
        {"Wi-Fi Setup", String("SSID: ") + network::wifi::AP_SSID,
         String("Pass: ") + network::wifi::AP_PASSWORD,
         WiFi.softAPIP().toString()});
  };
  callbacks.on_close = [](bool connected) {
    // Keep the credentials entered in the portal
    if (connected) storage::configuration::Configuration::save();
    display::matrix::Matrix::clearScreen();
  };
  _wifi = std::make_unique<network::wifi::Wifi>(std::move(callbacks));
  _wifi->begin();
  Serial.println("SystemController: Initialized the hardware components!");
  Serial.println("SystemController: Initializing the software components...");
  _ota.begin();
  // setup MQTT if enabled
  if (network::mqtt::enable) setupMqtt();

  // setup the clock
  _clock = std::make_unique<application::clock::Clock>();
  // setup the gif library
  _gif.begin(LITTLE_ENDIAN_PIXELS);
  // setup the configuration menu
  setupCli();
  Serial.println("SystemController: Initialized the software components!");
  delay(2000);

  // The setup screen of the Wi-Fi portal takes precedence
  if (!_sd->available() && !_wifi->portalActive()) {
    display::matrix::Matrix::printLines({"No SD card", "Clock only"});
    delay(NOTICE_MS);
  }
}

/**
 * Function used to set up MQTT and connect it to the matrix and the updater
 */
void SystemController::setupMqtt() {
  network::mqtt::Handlers handlers;
  handlers.on_light_command =
      [this](const network::mqtt::LightCommand& command) {
        if (command.brightness &&
            *command.brightness != display::matrix::brightness) {
          display::matrix::Matrix::setBrightness(*command.brightness);
          // Saving is deferred to bundle changes, e.g. from a slider
          _config_changed_at = millis();
          _config_dirty = true;
        }
        if (command.on) {
          display::matrix::Matrix::setPower(*command.on);
        }
      };
  handlers.get_light_state = []() {
    return network::mqtt::LightState{display::matrix::Matrix::isOn(),
                                     display::matrix::brightness};
  };
  handlers.on_install = [this]() { _ota.requestInstall(); };
  handlers.get_update_state = [this]() {
    const update::ota::Release& latest = _ota.latest();
    network::mqtt::UpdateState state;
    state.installed_version = update::ota::Ota::installedVersion();
    // Until a release is known the installed version is the latest one
    state.latest_version =
        _ota.checked() ? latest.version : state.installed_version;
    state.release_url = latest.release_url;
    state.release_summary = latest.release_summary;
    state.in_progress = _ota.installing();
    state.percentage = _ota.progress();
    return state;
  };
  _mqtt = std::make_unique<network::mqtt::Mqtt>(std::move(handlers));
  _mqtt->begin();
}

/**
 * Function used to set up the configuration menu and its actions
 */
void SystemController::setupCli() {
  std::vector<cli::Action> actions;
  actions.push_back(
      {"network/wifi/reset",
       "Forget the Wi-Fi credentials and reboot into the setup portal",
       [this]() {
         _wifi->reset();
         storage::configuration::Configuration::save();
         ESP.restart();
       },
       true});
  actions.push_back({"update/check", "Check for a new firmware",
                     [this]() {
                       if (!_ota.check()) {
                         Serial.println("Checking for a new firmware failed");
                       } else if (_ota.updateAvailable()) {
                         Serial.printf("Firmware %s is available\r\n",
                                       _ota.latest().version.c_str());
                       } else {
                         Serial.println("The firmware is up to date");
                       }
                       if (_mqtt) _mqtt->publishUpdateState();
                     },
                     false});
  actions.push_back({"update/install", "Install the latest firmware",
                     [this]() {
                       if (_ota.updateAvailable()) {
                         _ota.requestInstall();
                       } else {
                         Serial.println(
                             "No update available, run check "
                             "first");
                       }
                     },
                     true});

  _cli = std::make_unique<cli::Cli>(
      [this](const storage::settings::Setting& setting) {
        // Apply settings that take effect immediately
        auto* value = std::get_if<uint8_t*>(&setting.value);
        if (value && *value == &display::matrix::brightness) {
          display::matrix::Matrix::setBrightness(display::matrix::brightness);
          if (_mqtt) _mqtt->publishLightState();
        }
      },
      std::move(actions));
  _cli->begin();
}

/**
 * Function used for loading and displaying the given GIF file
 */
bool SystemController::displayGif(const String& filename) {
  if (!_gif.open(filename.c_str(), storage::sd::Sd::openGifFile,
                 storage::sd::Sd::closeGifFile, storage::sd::Sd::readGifFile,
                 storage::sd::Sd::seekGifFile,
                 display::matrix::Matrix::drawGif)) {
    Serial.println("SystemController: Failed to read GIF!");
    return false;
  } else {
    display::matrix::Matrix::clearScreen();
    // Keep the system parts running between frames and stop playback
    // as soon as the matrix gets turned off or an update is requested
    while (keepRendering() && _gif.playFrame(true, NULL)) {
      systemLoop();
    }
    _gif.close();
  }
  return true;
}

/**
 * Function used to wait while keeping the system parts running,
 * returns early if the rendering should stop
 */
void SystemController::wait(uint32_t ms) {
  uint32_t start = millis();
  while (millis() - start < ms && keepRendering()) {
    systemLoop();
    delay(10);
  }
}

/**
 * Function used to check if the applications should keep rendering,
 * they stop if the matrix is turned off, an update is requested or the
 * Wi-Fi setup portal is open
 */
bool SystemController::keepRendering() {
  // The setup screen of the Wi-Fi portal stays visible while it is open
  return display::matrix::Matrix::isOn() && !_ota.installRequested() &&
         !_wifi->portalActive();
}

/**
 * Function used for running the application parts of the device
 */
void SystemController::applicationLoop() {
  // Installed here instead of the system loop, as that one also runs
  // nested inside the applications
  if (_ota.installRequested()) {
    runUpdate();
    return;
  }
  if (!keepRendering()) return;

  // Until the time is synced, e.g. without network, no date is shown
  bool synced = _clock->synced();
  display::matrix::Matrix::printClock(synced ? _clock->dateTime("H:i")
                                             : String("--:--"));
  wait(SHOW_APPLICATION_MS);
  if (!keepRendering()) return;
  if (synced) {
    display::matrix::Matrix::printDate(_clock->dateTime("d.M.Y"));
    wait(SHOW_APPLICATION_MS);
  }
  // GIFs are only available with a SD card
  if (!keepRendering() || !_sd->available()) return;
  String filename;
  if (!_sd->next(filename)) {
    Serial.println(
        "SystemController: Iterator exhausted, resetting the iterator!");
    _sd->resetIterator();
  } else {
    displayGif(filename);
  }
}

/**
 * Function used for running the system parts of the device
 */
void SystemController::systemLoop() {
  if (_cli) _cli->loop();
  if (_wifi) _wifi->loop();
  if (_mqtt) _mqtt->loop();

  if (_config_dirty && millis() - _config_changed_at >= SAVE_DELAY_MS) {
    _config_dirty = false;
    storage::configuration::Configuration::save();
  }

  // A new firmware is confirmed once it reached the network,
  // otherwise it gets rolled back after a timeout
  if (update::ota::Ota::pendingConfirm() && healthy()) {
    update::ota::Ota::confirm();
  }
  if (_ota.loop() && _mqtt) _mqtt->publishUpdateState();
}

/**
 * Function used to check if the device is fully operational
 */
bool SystemController::healthy() {
  if (WiFi.status() != WL_CONNECTED) return false;
  return !_mqtt || _mqtt->connected();
}

/**
 * Function used to install a requested update, runs from the main loop
 * after the applications stopped rendering. Restarts on success.
 */
void SystemController::runUpdate() {
  // Write pending configuration changes, they would be lost on restart
  if (_config_dirty) {
    _config_dirty = false;
    storage::configuration::Configuration::save();
  }

  int8_t last_published = -1;
  display::matrix::Matrix::printUpdateProgress(0);
  if (_mqtt) _mqtt->publishUpdateState();

  bool ok = _ota.install([this, &last_published](uint8_t percent) {
    display::matrix::Matrix::printUpdateProgress(percent);
    // Throttle the MQTT updates, publishing also keeps the connection alive
    if (_mqtt && percent - last_published >= 5) {
      last_published = percent;
      _mqtt->publishUpdateState();
    }
  });

  if (ok) {
    Serial.println("SystemController: Restarting into the new firmware...");
    delay(500);
    ESP.restart();
  }
  // Report the failed installation and resume the applications
  if (_mqtt) _mqtt->publishUpdateState();
  display::matrix::Matrix::clearScreen();
}
