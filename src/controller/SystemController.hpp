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

#include "application/Clock.hpp"
#include "cli/Cli.hpp"
#include "display/Matrix.hpp"
#include "network/Mqtt.hpp"
#include "network/Wifi.hpp"
#include "storage/Configuration.hpp"
#include "storage/Sd.hpp"
#include "update/Ota.hpp"

// Time each application is shown
constexpr uint32_t SHOW_APPLICATION_MS = 5000;
// Time without further changes before the configuration gets saved
constexpr uint32_t SAVE_DELAY_MS = 5000;

class SystemController {
 private:
  // Hardware components
  std::unique_ptr<storage::sd::Sd> _sd;
  std::unique_ptr<network::wifi::Wifi> _wifi;
  // Software components
  std::unique_ptr<application::clock::Clock> _clock;
  std::unique_ptr<network::mqtt::Mqtt> _mqtt;
  std::unique_ptr<cli::Cli> _cli;
  update::ota::Ota _ota;
  AnimatedGIF _gif;
  // Pending configuration changes
  bool _config_dirty = false;
  uint32_t _config_changed_at = 0;

  bool displayGif(const String& filename);
  void wait(uint32_t ms);
  bool keepRendering();
  void runUpdate();
  void setupMqtt();
  void setupCli();
  bool healthy();

 public:
  void begin();
  void applicationLoop();
  void systemLoop();
};
