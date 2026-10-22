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

#include <array>
#include <cstddef>
#include <cstdint>

namespace display::logo {

constexpr int16_t WIDTH = 128;
constexpr int16_t HEIGHT = 32;

constexpr uint16_t WHITE = 0xFFFF;
constexpr uint16_t RED = 0xF800;

// clang-format off
// The logo, '#' is drawn white, 'r' red and '.' is left off. Only used at
// compile time to create the bitmaps below, so it is not part of the firmware.
constexpr const char* ART[HEIGHT] = {
    "...............................##########.....##########...#####.....#####...##########.....########.....########..###########..",
    "...............................##########.....##########...#####.....#####...##########.....########.....########..###########..",
    "...............................##########.....##########...#####.....#####...##########.....########.....########..###########..",
    "...............................#############..##########...#####.....#####...#############..#####################..#############",
    "...............................#####..######.....#####.....#####.....#####...#####..######..#####################..#####...#####",
    "...............................#####..######.....#####.....#####.....#####...#####..######..#####################..#####...#####",
    "...............................#####..######.....#####.....#####.....#####...#####..######..#####################..#####...#####",
    "..rrrrrrrr........rrrrrrrr.....#####..######.....#####.....########..#####...#####..######..#####...#####..######..#####...#####",
    "..rrrrrrrr........rrrrrrrr.....#####..######.....#####.....########..#####...#####..######..#####...#####..######..#####...#####",
    "..rrrrrrrr........rrrrrrrr.....#####..######.....#####.....########..#####...#####..######..#####...#####..######..#####...#####",
    "..rrrrrrrr........rrrrrrrr.....#####..######.....#####.....########..#####...#####..######..#####...#####..######..#####...#####",
    "rrrrr...rrrrr..rrrrrr..rrrrr...#####..######.....#####.....########..#####...#####..######..#####...#####..######..#####...#####",
    "rrrrr...rrrrr..rrrrrr..rrrrr...#####..######.....#####.....########..#####...#####..######..#####...#####..######..#####...#####",
    "rrrrr...rrrrr..rrrrrr..rrrrr...#####..######.....#####.....########..#####...#####..######..#####...#####..######..#####...#####",
    "rrrrr...rrrrr..rrrrrr..........##########........#####.....#####..########...#####..######..#####...#####..######..#####...#####",
    "rrrrr...rrrrr..rrrrrr..........##########........#####.....#####..########...#####..######..#####...#####..######..#####...#####",
    "rrrrr...rrrrr..rrrrrr..........##########........#####.....#####..########...#####..######..#####...#####..######..#####...#####",
    "rrrrr...rrrrr..rrrrrr..........##########........#####.....#####..########...#####..######..#####...#####..######..#####...#####",
    "rrrrrrrrrrrrr.....rrrrrrrr.....#####.............#####.....#####..########...#####..######..#####...#####..######..#####...#####",
    "rrrrrrrrrrrrr.....rrrrrrrr.....#####.............#####.....#####..########...#####..######..#####...#####..######..#####...#####",
    "rrrrrrrrrrrrr.....rrrrrrrr.....#####.............#####.....#####..########...#####..######..#####...#####..######..#####...#####",
    "rrrrr..................rrrrr...#####.............#####.....#####.....#####...#####..######..#####...#####..######..#####...#####",
    "rrrrr..................rrrrr...#####.............#####.....#####.....#####...#####..######..#####...#####..######..#####...#####",
    "rrrrr..................rrrrr...#####.............#####.....#####.....#####...#####..######..#####...#####..######..#####...#####",
    "rrrrr..................rrrrr...#####.............#####.....#####.....#####...#####..######..#####...#####..######..#####...#####",
    "rrrrr...rrrrr..rrrrrr..rrrrr...#####.............#####.....#####.....#####...#####..######..#####...#####..######..#####...#####",
    "rrrrr...rrrrr..rrrrrr..rrrrr...#####.............#####.....#####.....#####...#####..######..#####...#####..######..#####...#####",
    "rrrrr...rrrrr..rrrrrr..rrrrr...#####.............#####.....#####.....#####...#####..######..#####...#####..######..#####...#####",
    "rrrrrrrrrrrrr..rrrrrrrrrrrrr...#####..........##########...#####.....#####...#############..#####...#####..######..#############",
    "..rrrrrrrr........rrrrrrrr.....#####..........##########...#####.....#####...##########.....#####...#####..######..###########..",
    "..rrrrrrrr........rrrrrrrr.....#####..........##########...#####.....#####...##########.....#####...#####..######..###########..",
    "..rrrrrrrr........rrrrrrrr.....#####..........##########...#####.....#####...##########.....#####...#####..######..###########..",
};
// clang-format on

constexpr size_t BYTES_PER_ROW = (WIDTH + 7) / 8;
using Bitmap = std::array<uint8_t, BYTES_PER_ROW * HEIGHT>;

/**
 * Function used to check that every row of the logo has the right width
 */
constexpr bool validRows() {
  for (int16_t y = 0; y < HEIGHT; y++) {
    int16_t length = 0;
    while (ART[y][length] != '\0') length++;
    if (length != WIDTH) return false;
  }
  return true;
}
static_assert(validRows(), "Every row of the logo needs WIDTH characters");

/**
 * Function used to extract the pixels of one colour into a 1 bit bitmap,
 * in the format used by Adafruit GFX (rows padded to bytes, MSB first)
 */
constexpr Bitmap layer(char colour) {
  Bitmap bitmap{};
  for (int16_t y = 0; y < HEIGHT; y++) {
    for (int16_t x = 0; x < WIDTH; x++) {
      if (ART[y][x] == colour) {
        bitmap[y * BYTES_PER_ROW + x / 8] |= 0x80 >> (x % 8);
      }
    }
  }
  return bitmap;
}

inline constexpr Bitmap WHITE_LAYER = layer('#');
inline constexpr Bitmap RED_LAYER = layer('r');

}  // namespace display::logo
