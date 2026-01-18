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

#include <AnimatedGIF.h>
#include <SPI.h>
#include <SdFat.h>

#include <memory>

#define MAX_GIF_FILES 100
#define MAX_FILENAME_LENGTH 64
#define MAX_DIRNAME_LENGTH 64
#define GIF_ROOT_PATH "/gif"
#define INDEX_FILENAME ".index"

// defines the SPI clock speed, this is optimized for stability
#define SPI_SPEED SD_SCK_MHZ(4)

#ifdef ARDUINO_ESP32_IOT_REDBOARD
#define SD_MOSI 23
#define SD_MISO 19
#define SD_CLK 18
#define SD_CS 5
#elif defined ARDUINO_METRO_ESP32S2
#define SD_MOSI
#define SD_MISO
#define SD_CLK
#define SD_CS
#elif defined ARDUINO_METRO_ESP32S3
#define SD_MOSI 42
#define SD_MISO 21
#define SD_CLK 39
#define SD_CS 45
#else
#define SD_MOSI
#define SD_MISO
#define SD_CLK
#define SD_CS
#endif

class SequentialIterator {
 private:
  SdFs &_sd;
  FsFile _dir;
  bool _isOpen;
  String _path;
  SequentialIterator *_child;

 public:
  SequentialIterator(SdFs &sd, String path)
      : _sd(sd), _isOpen(false), _child(nullptr), _path(path) {
    if (_dir.open(path.c_str())) {
      _isOpen = true;
    } else {
      Serial.print("SequentialIterator: Failed to open directory: ");
      Serial.println(path);
    }
  }
  ~SequentialIterator() {
    if (_child) {
      delete _child;
    }
    if (_isOpen) {
      _dir.close();
    }
  }
  bool next(String &file_path);
};

class IndexedIterator {
 private:
 public:
  void next();
};

// Global instances of sd card component objects
// Needed because of a limitation of the AnimationGIF library
inline SdFs sd;
inline FsFile file;

/**
 * Class used for initialization and low level access to
 * files on the SD Card. It also features functionality
 * for generating a index file.
 */
class Sd {
 private:
  String _gifFiles[MAX_GIF_FILES];
  int _gifCount = 0;
  std::unique_ptr<SequentialIterator> _sequentialIterator;
  std::unique_ptr<IndexedIterator> _indexedIterator;

  bool isGifFile(const char *filename);
  void indexGifFiles(SdFile dir, String pathPrefix = "");

 public:
  Sd();
  int generateFileIndex(const char *folderPath, const char *indexFilename);
  int loadFileIndex(const char *indexFilename);
  static bool openFile(String &filename, FsFile &file);
  static bool openFile(const char *filename, FsFile &file);
  static bool closeFile(FsFile &file);
  void resetIterator();
  bool next(String &filename);
  // Static functions for use with the AnimatedGIF library
  static void *openGifFile(const char *fname, int32_t *pSize);
  static void closeGifFile(void *pHandle);
  static int32_t readGifFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
  static int32_t seekGifFile(GIFFILE *pFile, int32_t iPosition);
};
