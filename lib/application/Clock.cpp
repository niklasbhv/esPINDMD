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

#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE "Europe/Berlin"
#define SYNC_TIMEOUT_S 60

Clock::Clock() {
  Serial.println("Clock: Initializing the clock component...");
  ezt::setServer(NTP_SERVER);
  Serial.println("Clock: Syncing with NTP server: " + String(NTP_SERVER));
  ezt::setInterval(SYNC_TIMEOUT_S);
  ezt::waitForSync();
  timezone.setLocation(TIMEZONE);
  Serial.println("Clock: Initialized the clock component!");
}

String Clock::dateTime(String format) {
  ezt::events();
  return timezone.dateTime(format); 
}