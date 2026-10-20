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

#include <WiFi.h>

namespace application::clock {

Clock::Clock() {
  Serial.println("Clock: Initializing the clock component...");
  ezt::setServer(ntp_server.c_str());
  Serial.printf("Clock: Syncing with NTP server: %s\r\n", ntp_server.c_str());
  ezt::setInterval(ntp_sync_timeout);
  // Without network the time is synced later on by ezt::events()
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Clock: No network, syncing in the background");
  } else if (!ezt::waitForSync(INITIAL_SYNC_TIMEOUT_S)) {
    Serial.println("Clock: Initial sync timed out, retrying in the background");
  }
  timezone.setLocation(ntp_timezone.c_str());
  Serial.println("Clock: Initialized the clock component!");
}

String Clock::dateTime(const String& format) {
  ezt::events();
  return timezone.dateTime(format);
}

/**
 * Function used to check if the time was synced at least once
 */
bool Clock::synced() {
  ezt::events();
  return ezt::timeStatus() != timeNotSet;
}

}  // namespace application::clock
