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
#include <ezTime.h>

namespace application::clock {

inline uint8_t colour_r = 231;
inline uint8_t colour_g = 103;
inline uint8_t colour_b = 3;
inline std::string ntp_server = "pool.ntp.org";
inline std::string ntp_timezone = "Europe/Berlin";
inline uint8_t ntp_sync_timeout = 60;

class Clock {
 private:
  Timezone timezone;

 public:
  Clock();
  String dateTime(String format);
};

}  // namespace application::clock
