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

#include <string>

namespace application::clock {

// POSIX timezone of central Europe, used by default
constexpr const char* DEFAULT_TIMEZONE = "CET-1CEST,M3.5.0,M10.5.0/3";

inline uint8_t colour_r = 231;
inline uint8_t colour_g = 103;
inline uint8_t colour_b = 3;
inline std::string ntp_server = "pool.ntp.org";
// POSIX timezone string, see https://github.com/nayarsystems/posix_tz_db
inline std::string ntp_timezone = DEFAULT_TIMEZONE;
// Minutes between two syncs with the NTP server
inline uint16_t ntp_interval = 60;

/**
 * The clock keeps the system time in sync using SNTP, which runs in the
 * background and starts syncing as soon as the network is available.
 */
class Clock {
 private:
  // Copy of the server, SNTP only keeps a pointer to it
  std::string _server;

 public:
  void begin();
  static void applyTimezone();
  bool synced() const;
  String format(const char* format) const;
};

}  // namespace application::clock
