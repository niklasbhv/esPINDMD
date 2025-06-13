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
#define SYNC_TIMEOUT_S 60

Clock::Clock() {
  Serial.println("Initializing the clock");
  setServer(NTP_SERVER);
  Serial.println("Syncing with NTP server: " + String(NTP_SERVER));
  waitForSync(SYNC_TIMEOUT_S);
}