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
#include <Wire.h>

namespace sensor::light {

// Use the light sensor if one is connected
inline bool enable = true;
// Defaults to the I2C pins of the board, which are the pins of the
// STEMMA QT / Qwiic connector on the supported boards
inline int8_t pins_sda = SDA;
inline int8_t pins_scl = SCL;
// Ambient light at which the full brightness is used
inline uint16_t full_lux = 300;

// The BH1750 uses 0x23 with ADDR low (default) and 0x5C with ADDR high
constexpr uint8_t ADDRESSES[] = {0x23, 0x5C};
constexpr uint8_t CMD_POWER_ON = 0x01;
// Continuous measurements with a resolution of 1 lux, takes 120 ms each
constexpr uint8_t CMD_CONTINUOUS_HIGH_RES = 0x10;
// Raw values are divided by this factor to get lux, see the datasheet
constexpr float LUX_FACTOR = 1.2f;
constexpr uint32_t I2C_FREQUENCY = 100000;
constexpr uint32_t READ_INTERVAL_MS = 1000;
// Time between attempts to reach a sensor that stopped responding
constexpr uint32_t RETRY_INTERVAL_MS = 10000;
// Weight of a new reading, smooths out short changes, e.g. shadows
constexpr float SMOOTHING = 0.2f;
// Minimum change of the level before it is applied, avoids flickering
constexpr uint8_t LEVEL_HYSTERESIS = 3;

/**
 * Optional BH1750 ambient light sensor, e.g. the Adafruit BH1750 connected
 * to the STEMMA QT port. Provides a level for dimming the matrix, from
 * 0 in the dark up to 255 at full_lux and above.
 */
class LightSensor {
 private:
  uint8_t _address = 0;
  bool _responding = false;
  // Smoothed ambient light, negative until the first reading
  float _lux = -1;
  uint8_t _level = 255;
  uint32_t _last_attempt = 0;

  bool configure();
  bool read(float& lux);
  uint8_t levelFor(float lux) const;

 public:
  bool begin();
  bool loop();
  bool responding() const;
  float lux() const;
  uint8_t level() const;
};

}  // namespace sensor::light
