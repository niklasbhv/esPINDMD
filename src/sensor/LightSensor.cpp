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

#include "LightSensor.hpp"

#include <cmath>

namespace sensor::light {

/**
 * Function used to send a single command byte to the sensor
 */
static bool command(uint8_t address, uint8_t cmd) {
  Wire.beginTransmission(address);
  Wire.write(cmd);
  return Wire.endTransmission() == 0;
}

/**
 * Function used to search for the sensor on the I2C bus. If no sensor is
 * found, the pins are released again so they stay usable otherwise.
 *
 * @return true if a sensor was found
 */
bool LightSensor::begin() {
  Serial.println("LightSensor: Initializing the light sensor component...");
  if (!Wire.begin(pins_sda, pins_scl, I2C_FREQUENCY)) {
    Serial.println("LightSensor: Failed to start the I2C bus");
    return false;
  }
  for (uint8_t address : ADDRESSES) {
    _address = address;
    if (configure()) {
      Serial.printf("LightSensor: Found a BH1750 at 0x%02X\r\n", address);
      _last_attempt = millis();
      return true;
    }
  }
  Serial.println("LightSensor: No light sensor found, continuing without it");
  Wire.end();
  return false;
}

/**
 * Function used to power on the sensor and start continuous measurements
 */
bool LightSensor::configure() {
  _responding = command(_address, CMD_POWER_ON) &&
                command(_address, CMD_CONTINUOUS_HIGH_RES);
  return _responding;
}

bool LightSensor::read(float& lux) {
  if (Wire.requestFrom(_address, static_cast<uint8_t>(2)) != 2) return false;
  uint16_t raw = Wire.read() << 8;
  raw |= Wire.read();
  lux = raw / LUX_FACTOR;
  return true;
}

/**
 * Function used to map the ambient light to a level, logarithmic as the
 * perceived brightness follows the light roughly logarithmically
 */
uint8_t LightSensor::levelFor(float lux) const {
  float level = log1pf(lux) / log1pf(std::max<uint16_t>(full_lux, 1));
  return static_cast<uint8_t>(std::clamp(level, 0.0f, 1.0f) * 255.0f + 0.5f);
}

/**
 * Function used to read the sensor periodically. If the sensor stops
 * responding, e.g. it was unplugged, the full level is used until it
 * responds again. Needs to be called frequently.
 *
 * @return true if there is a new reading or the sensor stopped responding
 */
bool LightSensor::loop() {
  uint32_t interval = _responding ? READ_INTERVAL_MS : RETRY_INTERVAL_MS;
  if (millis() - _last_attempt < interval) return false;
  _last_attempt = millis();

  // A sensor that lost power needs to be configured again
  if (!_responding) {
    if (!configure()) return false;
    Serial.println("LightSensor: Light sensor is responding again");
    // The first measurement is not ready yet
    return false;
  }

  float lux;
  if (!read(lux)) {
    Serial.println("LightSensor: Light sensor stopped responding");
    _responding = false;
    _lux = -1;
    _level = 255;
    return true;
  }
  _lux = _lux < 0 ? lux : _lux + (lux - _lux) * SMOOTHING;

  uint8_t level = levelFor(_lux);
  // The ends are always applied, so the full range is reachable
  bool end = (level == 0 || level == 255) && level != _level;
  if (end || abs(level - _level) >= LEVEL_HYSTERESIS) _level = level;
  return true;
}

bool LightSensor::responding() const { return _responding && _lux >= 0; }

float LightSensor::lux() const { return _lux; }

uint8_t LightSensor::level() const { return _level; }

}  // namespace sensor::light
