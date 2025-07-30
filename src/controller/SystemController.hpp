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
#include "display/Logo.hpp"
#include "display/Matrix.hpp"
#include "network/Mqtt.hpp"
#include "network/Wifi.hpp"
#include "storage/Sd.hpp"

#define SHOW_CLOCK_MS 5000

class SystemController {
 private:
  // Hardware components
  std::unique_ptr<Sd> _sd;
  std::unique_ptr<Wifi> _wifi;
  // Software components
  std::unique_ptr<Clock> _clock;
  std::unique_ptr<Mqtt> _mqtt;
  std::unique_ptr<Cli> _cli;
  AnimatedGIF _gif;

  void displayLogo();
  bool displayGif(String& filename);

 public:
  SystemController();
  void begin();
  void loop();
};