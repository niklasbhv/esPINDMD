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

#include <SPI.h>
#include <SdFat.h>

// defines compatibility for FAT16, FAT32 and exFAT
#define SD_FAT_TYPE 3

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
#if SD_FAT_TYPE == 0
  SdFat _sd;
  File _file;
#elif SD_FAT_TYPE == 1
  SdFat32 _sd;
  File32 _file;
#elif SD_FAT_TYPE == 2
  SdExFat _sd;
  ExFile _file;
#elif SD_FAT_TYPE == 3
  SdFs _sd;
  FsFile _file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

 public:
  Sd();
  int generateFileIndex(const char* folderPath, const char* indexFilename);
  int loadFileIndex(const char* indexFilename);
};