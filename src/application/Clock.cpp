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

#include "Clock.hpp"

namespace application::clock {

Clock::Clock() {
  Serial.println("Clock: Initializing the clock component...");
  ezt::setServer(ntp_server.c_str());
  Serial.println("Clock: Syncing with NTP server: " + String(NTP_SERVER));
  ezt::setInterval(ntp_sync_timeout);
  ezt::waitForSync();
  timezone.setLocation(ntp_timezone.c_str());
  Serial.println("Clock: Initialized the clock component!");
}

String Clock::dateTime(String format) {
  ezt::events();
  return timezone.dateTime(format);
}

}  // namespace application::clock
