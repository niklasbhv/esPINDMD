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

#include "cli/Cli.hpp"
#include "application/Clock.hpp"
#include "display/Matrix.hpp"
#include "network/Mqtt.hpp"
#include "storage/Sd.hpp"
#include "network/Wifi.hpp"
#include "storage/GifLoader.hpp"

class SystemController {
 private:
  // Hardware components
  std::unique_ptr<Matrix> _matrix;
  std::unique_ptr<Sd> _sd;
  std::unique_ptr<Wifi> _wifi;
  // Software components
  std::unique_ptr<Clock> _clock;
  std::unique_ptr<Mqtt> _mqtt;
  std::unique_ptr<Cli> _cli;
  std::unique_ptr<SequentialIterator> _sequentialIterator;
  std::unique_ptr<SequentialIterator> _indexedIterator;

 public:
  SystemController();
  void begin();
  void loop();
};