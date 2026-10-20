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

#ifndef DISABLE_FS_H_WARNING
#define DISABLE_FS_H_WARNING  // Disable warning for type File not defined.
#endif                        // DISABLE_FS_H_WARNING

#include <AnimatedGIF.h>
#include <SPI.h>
#include <SdFat.h>

#include <functional>
#include <memory>

namespace storage::sd {

// Defaults to the SPI pins of the board
inline uint8_t pins_miso = MISO;
inline uint8_t pins_mosi = MOSI;
inline uint8_t pins_clk = SCK;
inline uint8_t pins_cs = SS;

// Play the GIFs in random order instead of the order of the index
inline bool play_random = true;

// Maximum length of a FAT file name including the terminator
constexpr size_t MAX_FILENAME_LENGTH = 256;
// Directory containing the GIF files, including all sub directories
constexpr const char* GIF_ROOT_PATH = "/gif";
// Index of all GIF files, hidden so it is skipped while indexing
constexpr const char* INDEX_PATH = "/gif/.index";
constexpr const char* INDEX_TMP_PATH = "/gif/.index.tmp";
// Every path is stored as a fixed size record, so the n-th path is found
// with a single seek
constexpr size_t INDEX_RECORD_LENGTH = 256;
// Number of recently played GIFs that are not picked again randomly
constexpr size_t RECENT_COUNT = 16;
// SPI clock speed, this is optimized for stability
constexpr uint32_t SPI_CLOCK = SD_SCK_MHZ(4);

/**
 * Iterates over all GIF files of a directory and its sub directories,
 * hidden files (starting with a dot) are skipped. Used to build the index.
 */
class SequentialIterator {
 private:
  FsFile _dir;
  String _path;
  bool _is_open = false;
  std::unique_ptr<SequentialIterator> _child;

 public:
  explicit SequentialIterator(const String& path);
  bool next(String& file_path);
};

using IndexProgress = std::function<void(uint32_t count)>;

/**
 * Index of all GIF files on the SD card, used to pick the next GIF either
 * in order or randomly without keeping a list of all files in memory
 */
class GifIndex {
 private:
  FsFile _file;
  uint32_t _count = 0;
  uint32_t _position = 0;
  // Ring buffer of recently played records
  uint32_t _recent[RECENT_COUNT];
  size_t _recent_size = 0;
  size_t _recent_next = 0;

  bool read(uint32_t record, String& path);
  bool recentlyPlayed(uint32_t record) const;
  void remember(uint32_t record);

 public:
  static bool build(const IndexProgress& progress);
  bool open();
  uint32_t count() const;
  bool next(String& path);
  void rewind();
};

// Global instances of sd card component objects
// Needed because of a limitation of the AnimationGIF library
inline SdFs sd;
inline FsFile file;
// Whether a SD card was found
inline bool mounted = false;

/**
 * Class used for initialization and low level access to
 * files on the SD Card. The device keeps running without a SD card,
 * only the GIF playback is not available then.
 */
class Sd {
 private:
  GifIndex _index;
  // Whether the default pins of the board were used as a fallback
  bool _default_pins = false;

 public:
  bool begin();
  bool restoreWorkingPins();
  bool indexGifs(const IndexProgress& progress);
  uint32_t gifCount() const;
  static bool available();
  static bool openFile(const char* filename, FsFile& file);
  static bool openFileForWrite(const char* filename, FsFile& file);
  static bool closeFile(FsFile& file);
  static bool exists(const char* filename);
  static bool replaceFile(const char* source, const char* target);
  void resetIterator();
  bool next(String& filename);
  // Static functions for use with the AnimatedGIF library
  static void* openGifFile(const char* fname, int32_t* pSize);
  static void closeGifFile(void* pHandle);
  static int32_t readGifFile(GIFFILE* pFile, uint8_t* pBuf, int32_t iLen);
  static int32_t seekGifFile(GIFFILE* pFile, int32_t iPosition);
};

}  // namespace storage::sd
