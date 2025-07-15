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

#define MAX_GIF_FILES 100

// defines the SPI clock speed, this is optimized for stability
#define SPI_SPEED SD_SCK_MHZ(4)

#define MAX_FILENAME_LENGTH 100

#define SD_MOSI 6
#define SD_MISO 5
#define SD_CLK 7
#define SD_CS 4

/**
 * Class used for initialization and low level access to
 * files on the SD Card. It also features functionality
 * for generating a index file.
 */
class Sd {
 private:
  String _gifFiles[MAX_GIF_FILES];
  int _gifCount = 0;
  SdFs _sd;

 public:
  Sd();
  int generateFileIndex(const char* folderPath, const char* indexFilename);
  int loadFileIndex(const char* indexFilename);
  bool openFile(String& filename, FsFile& file);
  bool closeFile(FsFile& file);
};