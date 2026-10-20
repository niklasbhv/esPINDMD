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

#include "Sd.hpp"

namespace storage::sd {

/**
 * Function used to check if a file is a GIF file that should be played
 */
static bool isGifFile(const char* filename) {
  // Skip hidden files, e.g. the ._ files created by macOS
  if (filename[0] == '.') return false;
  const char* extension = strrchr(filename, '.');
  return extension && strcasecmp(extension, ".gif") == 0;
}

/**
 * Function used to mount the SD card using the given SPI pins
 */
static bool mount(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t cs) {
  // The bus needs to be restarted to change the pins, SdFat keeps them as
  // the bus is already started
  SPI.end();
  SPI.begin(clk, miso, mosi, cs);
  return sd.begin(SdSpiConfig(cs, DEDICATED_SPI, SPI_CLOCK));
}

/**
 * Function used to mount the SD card. If the configured pins fail, the
 * default pins of the board are tried, as wrong pins could otherwise only
 * be fixed using the CLI.
 *
 * @return true if a SD card was found
 */
bool Sd::begin() {
  Serial.println("SD: Initializing the sd component...");
  Serial.printf("SD: SPI pins MISO %u, MOSI %u, CLK %u, CS %u\r\n", pins_miso,
                pins_mosi, pins_clk, pins_cs);

  mounted = mount(pins_clk, pins_miso, pins_mosi, pins_cs);
  bool defaults = pins_clk == SCK && pins_miso == MISO && pins_mosi == MOSI &&
                  pins_cs == SS;
  if (!mounted && !defaults) {
    Serial.println("SD: Failed, retrying with the default pins of the board");
    mounted = mount(SCK, MISO, MOSI, SS);
    if (mounted) {
      // Keep the working pins, so they are stored on the next save
      Serial.println("SD: Default pins work, the configured ones are wrong");
      pins_clk = SCK;
      pins_miso = MISO;
      pins_mosi = MOSI;
      pins_cs = SS;
    }
  }

  if (!mounted) {
    Serial.println("SD: No SD card found, continuing without GIFs");
    return false;
  }
  _sequential_iterator = std::make_unique<SequentialIterator>(GIF_ROOT_PATH);
  Serial.println("SD: Initialized the sd component!");
  return true;
}

bool Sd::available() { return mounted; }

bool Sd::openFile(const char* filename, FsFile& file) {
  return file.open(filename, O_READ);
}

/**
 * Opens a file for writing, the file is created or truncated
 */
bool Sd::openFileForWrite(const char* filename, FsFile& file) {
  return file.open(filename, O_WRONLY | O_CREAT | O_TRUNC);
}

bool Sd::closeFile(FsFile& file) { return file.close(); }

bool Sd::exists(const char* filename) { return sd.exists(filename); }

/**
 * Replaces the target file with the source file, FAT does not
 * support renaming onto an existing file so the target is removed first
 */
bool Sd::replaceFile(const char* source, const char* target) {
  if (sd.exists(target) && !sd.remove(target)) return false;
  return sd.rename(source, target);
}

void Sd::resetIterator() {
  if (!mounted) return;
  _sequential_iterator = std::make_unique<SequentialIterator>(GIF_ROOT_PATH);
}

bool Sd::next(String& file_path) {
  file_path.clear();
  if (!mounted) return false;
  return _sequential_iterator->next(file_path);
}

SequentialIterator::SequentialIterator(const String& path) : _path(path) {
  _is_open = _dir.open(path.c_str());
  if (!_is_open) {
    Serial.print("SequentialIterator: Failed to open directory: ");
    Serial.println(path);
  }
}

bool SequentialIterator::next(String& file_path) {
  // If we are in a subdirectory, check that first
  if (_child) {
    if (_child->next(file_path)) return true;
    // Subdirectory done, continue with this directory
    _child.reset();
  }

  if (!_is_open) return false;

  FsFile entry;
  char name[MAX_FILENAME_LENGTH];
  while (entry.openNext(&_dir, O_RDONLY)) {
    entry.getName(name, sizeof(name));
    bool is_dir = entry.isDir();
    entry.close();

    if (is_dir && name[0] != '.') {
      // Continue with the first file of the subdirectory
      _child = std::make_unique<SequentialIterator>(_path + "/" + name);
      if (_child->next(file_path)) return true;
      _child.reset();
    } else if (!is_dir && isGifFile(name)) {
      file_path = _path + "/" + name;
      return true;
    }
  }

  // Done with this directory
  _dir.close();
  _is_open = false;
  return false;
}

void* Sd::openGifFile(const char* fname, int32_t* pSize) {
  file = sd.open(fname);
  if (file) {
    *pSize = file.size();
    return &file;
  }
  return nullptr;
}

void Sd::closeGifFile(void* pHandle) {
  FsFile* file = static_cast<FsFile*>(pHandle);
  if (file != nullptr) file->close();
}

int32_t Sd::readGifFile(GIFFILE* pFile, uint8_t* pBuf, int32_t iLen) {
  int32_t iBytesRead = iLen;
  FsFile* file = static_cast<FsFile*>(pFile->fHandle);
  // Note: If you read a file all the way to the last byte, seek() stops working
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos - 1;  // <-- ugly work-around
  if (iBytesRead <= 0) return 0;
  iBytesRead = static_cast<int32_t>(file->read(pBuf, iBytesRead));
  pFile->iPos = file->position();
  return iBytesRead;
}

int32_t Sd::seekGifFile(GIFFILE* pFile, int32_t iPosition) {
  FsFile* file = static_cast<FsFile*>(pFile->fHandle);
  file->seek(iPosition);
  pFile->iPos = static_cast<int32_t>(file->position());
  return pFile->iPos;
}

}  // namespace storage::sd
