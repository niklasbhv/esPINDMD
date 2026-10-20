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

#include <esp_sntp.h>
#include <time.h>

namespace application::clock {

// Set by SNTP once the time was synced
static volatile bool time_synced = false;

static void onTimeSynced(struct timeval*) {
  if (!time_synced) Serial.println("Clock: Synced the time");
  time_synced = true;
}

void Clock::begin() {
  Serial.println("Clock: Initializing the clock component...");
  // Older configurations used timezone names, which are not supported
  if (ntp_timezone.find('/') != std::string::npos) {
    Serial.printf("Clock: %s is not a POSIX timezone, using %s instead\r\n",
                  ntp_timezone.c_str(), DEFAULT_TIMEZONE);
    ntp_timezone = DEFAULT_TIMEZONE;
  }

  _server = ntp_server;
  Serial.printf("Clock: Syncing with NTP server: %s\r\n", _server.c_str());
  sntp_set_time_sync_notification_cb(onTimeSynced);
  sntp_set_sync_interval(ntp_interval * 60UL * 1000UL);
  // Sets the timezone and starts syncing in the background
  configTzTime(ntp_timezone.c_str(), _server.c_str());
  Serial.println("Clock: Initialized the clock component!");
}

/**
 * Function used to apply a changed timezone immediately
 */
void Clock::applyTimezone() {
  setenv("TZ", ntp_timezone.c_str(), 1);
  tzset();
}

/**
 * Function used to check if the time was synced at least once
 */
bool Clock::synced() const { return time_synced; }

/**
 * Function used to format the local time
 *
 * @param format Format as used by strftime, e.g. "%H:%M"
 */
String Clock::format(const char* format) const {
  time_t now = time(nullptr);
  struct tm local;
  localtime_r(&now, &local);
  char buffer[32];
  strftime(buffer, sizeof(buffer), format, &local);
  return String(buffer);
}

}  // namespace application::clock
