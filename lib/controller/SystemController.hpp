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

#include "Cli.hpp"
#include "Clock.hpp"
#include "Gif.hpp"
#include "Matrix.hpp"
#include "Mqtt.hpp"
#include "Sd.hpp"
#include "Wifi.hpp"

class SystemController {
 private:
  Clock _clock;
  Gif _gif;
  Sd _sd;
  Matrix _matrix;
  Wifi _wifi;
  Mqtt _mqtt(const char* mqtt_server, uint16_t mqtt_port);
  Cli _cli;

 public:
  SystemController();
};