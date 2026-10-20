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

#include <memory>

namespace storage::sd {

// Defaults to the SPI pins of the board
inline uint8_t pins_miso = MISO;
inline uint8_t pins_mosi = MOSI;
inline uint8_t pins_clk = SCK;
inline uint8_t pins_cs = SS;

constexpr size_t MAX_FILENAME_LENGTH = 64;
// Directory containing the GIF files, including all sub directories
constexpr const char* GIF_ROOT_PATH = "/gif";
// SPI clock speed, this is optimized for stability
constexpr uint32_t SPI_CLOCK = SD_SCK_MHZ(4);

/**
 * Iterates over all GIF files of a directory and its sub directories,
 * hidden files (starting with a dot) are skipped
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

// Global instances of sd card component objects
// Needed because of a limitation of the AnimationGIF library
inline SdFs sd;
inline FsFile file;

/**
 * Class used for initialization and low level access to
 * files on the SD Card.
 */
class Sd {
 private:
  std::unique_ptr<SequentialIterator> _sequential_iterator;

 public:
  Sd();
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
