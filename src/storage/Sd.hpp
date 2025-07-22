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
#define GIF_ROOT_PATH "/gif"

// defines the SPI clock speed, this is optimized for stability
#define SPI_SPEED SD_SCK_MHZ(4)

#define MAX_FILENAME_LENGTH 100

#define SD_MOSI 6
#define SD_MISO 5
#define SD_CLK 7
#define SD_CS 4

class SequentialIterator {
 private:
  SdFs &_sd;
  FsFile _dir;
  bool _isOpen;
  SequentialIterator *_child;

  // Helper: get path of current directory (if possible)
  const char *_dirName() {
    static char name[64];
    _dir.getName(name, sizeof(name));
    return name;
  }

 public:
  SequentialIterator(SdFs &sd, const char *path)
      : _sd(sd), _isOpen(false), _child(nullptr) {
    if (_dir.open(path)) {
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

 public:
  Sd();
  int generateFileIndex(const char *folderPath, const char *indexFilename);
  int loadFileIndex(const char *indexFilename);
  bool openFile(String &filename, FsFile &file);
  bool closeFile(FsFile &file);
  bool next(String &filename);
  // Static functions for use with the AnimatedGIF library
  static void *openGifFile(const char *fname, int32_t *pSize);
  static void closeGifFile(void *pHandle);
  static int32_t readGifFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
  static int32_t seekGifFile(GIFFILE *pFile, int32_t iPosition);
};
