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
      Serial.println("SD: Default pins work, the configured ones are wrong");
      _default_pins = true;
      restoreWorkingPins();
    }
  }

  if (!mounted) {
    Serial.println("SD: No SD card found, continuing without GIFs");
    return false;
  }
  Serial.println("SD: Initialized the sd component!");
  return true;
}

bool Sd::available() { return mounted; }

/**
 * Function used to set the pins back to the working default pins, if they
 * were used as a fallback. Required after importing a configuration file,
 * as it may contain the wrong pins again.
 *
 * @return true if the pins were changed
 */
bool Sd::restoreWorkingPins() {
  if (!_default_pins) return false;
  bool changed = pins_clk != SCK || pins_miso != MISO || pins_mosi != MOSI ||
                 pins_cs != SS;
  pins_clk = SCK;
  pins_miso = MISO;
  pins_mosi = MOSI;
  pins_cs = SS;
  return changed;
}

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

/**
 * Function used to open the GIF index, it is only rebuilt if the GIF files
 * changed since it was built. If building fails, the previous index is used.
 *
 * @param progress Invoked with the number of indexed files while building
 */
bool Sd::indexGifs(const IndexProgress& progress) {
  if (!mounted) return false;
  if (!GifIndex::upToDate()) GifIndex::build(progress);
  return _index.open();
}

uint32_t Sd::gifCount() const { return _index.count(); }

/**
 * Function used to restart the playback in order from the beginning
 */
void Sd::resetIterator() { _index.rewind(); }

/**
 * Function used to get the next GIF, either randomly or in order
 *
 * @return false if there are no GIFs or all of them were played in order
 */
bool Sd::next(String& file_path) {
  file_path.clear();
  if (!mounted) return false;
  return _index.next(file_path);
}

/**
 * Function used to add a directory entry to the signature, only fields
 * that are stored in the entry itself are used so no extra reads are needed
 */
void IndexSignature::add(FsFile& entry) {
  auto mix = [this](const void* data, size_t length) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < length; i++) {
      hash = (hash ^ bytes[i]) * 16777619u;
    }
  };
  uint8_t is_dir = entry.isDir();
  uint64_t size = entry.fileSize();
  uint32_t dir_index = entry.dirIndex();
  uint32_t cluster = entry.firstCluster();
  uint16_t date = 0;
  uint16_t time = 0;
  entry.getModifyDateTime(&date, &time);
  mix(&is_dir, sizeof(is_dir));
  mix(&size, sizeof(size));
  mix(&dir_index, sizeof(dir_index));
  mix(&cluster, sizeof(cluster));
  mix(&date, sizeof(date));
  mix(&time, sizeof(time));
  entries++;
}

/**
 * Function used to add all entries of a directory and its sub directories
 * to the signature
 */
static void scanDirectory(FsFile& dir, IndexSignature& signature) {
  FsFile entry;
  while (entry.openNext(&dir, O_RDONLY)) {
    signature.add(entry);
    // The opened entry is used directly, so the name is never needed
    if (entry.isDir()) scanDirectory(entry, signature);
    entry.close();
  }
}

/**
 * Function used to read the signature stored in the header of the index
 *
 * @return false if there is no valid index
 */
bool GifIndex::readSignature(IndexSignature& signature) {
  FsFile index;
  if (!index.open(INDEX_PATH, O_RDONLY)) return false;
  IndexHeader header;
  bool ok = index.read(&header, sizeof(header)) == sizeof(header) &&
            memcmp(header.magic, INDEX_MAGIC, sizeof(INDEX_MAGIC)) == 0 &&
            index.size() % INDEX_RECORD_LENGTH == 0;
  index.close();
  if (ok) signature = header.signature;
  return ok;
}

/**
 * Function used to build the signature of the GIF directory, this only
 * reads the directory entries and is much faster than building the index
 */
IndexSignature GifIndex::scan() {
  IndexSignature signature;
  FsFile dir;
  if (dir.open(GIF_ROOT_PATH, O_RDONLY)) {
    scanDirectory(dir, signature);
    dir.close();
  }
  return signature;
}

/**
 * Function used to check whether the index matches the current GIF files
 */
bool GifIndex::upToDate() {
  IndexSignature stored;
  if (!readSignature(stored)) {
    Serial.println("SD: No valid GIF index found");
    return false;
  }
  Serial.println("SD: Checking the GIF index...");
  uint32_t start = millis();
  bool up_to_date = scan() == stored;
  Serial.printf("SD: Checked %lu entries in %lu ms, the index is %s\r\n",
                static_cast<unsigned long>(stored.entries),
                static_cast<unsigned long>(millis() - start),
                up_to_date ? "up to date" : "outdated");
  return up_to_date;
}

/**
 * Function used to write the paths of all GIFs into the index
 */
bool GifIndex::build(const IndexProgress& progress) {
  Serial.println("SD: Indexing GIFs...");
  uint32_t start = millis();
  // Removing the old index changes the GIF directory, so it is done before
  // the signature is taken
  if (sd.exists(LEGACY_INDEX_PATH)) sd.remove(LEGACY_INDEX_PATH);
  if (sd.exists(LEGACY_INDEX_TMP_PATH)) sd.remove(LEGACY_INDEX_TMP_PATH);
  IndexSignature signature = scan();
  FsFile index;
  if (!index.open(INDEX_TMP_PATH, O_WRONLY | O_CREAT | O_TRUNC)) {
    Serial.println("SD: Failed to create the index");
    return false;
  }

  SequentialIterator iterator(GIF_ROOT_PATH);
  String path;
  uint32_t count = 0;
  char record[INDEX_RECORD_LENGTH];
  // Reserve the header, it is written once the signature is known
  memset(record, 0, sizeof(record));
  bool ok = index.write(record, sizeof(record)) == sizeof(record);
  if (progress) progress(count);
  while (ok && iterator.next(path)) {
    if (path.length() >= INDEX_RECORD_LENGTH) {
      Serial.printf("SD: Skipping %s, the path is too long\r\n", path.c_str());
      continue;
    }
    memset(record, 0, sizeof(record));
    memcpy(record, path.c_str(), path.length());
    if (index.write(record, sizeof(record)) != sizeof(record)) {
      ok = false;
      break;
    }
    count++;
    if (progress) progress(count);
  }

  IndexHeader header;
  memcpy(header.magic, INDEX_MAGIC, sizeof(INDEX_MAGIC));
  header.signature = signature;
  ok = ok && index.seekSet(0) &&
       index.write(&header, sizeof(header)) == sizeof(header);

  // Closing flushes the file, a failure means the data is not on the card
  ok = index.close() && ok;
  if (!ok || !Sd::replaceFile(INDEX_TMP_PATH, INDEX_PATH)) {
    Serial.println("SD: Failed to write the index");
    return false;
  }
  Serial.printf("SD: Indexed %lu GIFs in %lu ms\r\n",
                static_cast<unsigned long>(count),
                static_cast<unsigned long>(millis() - start));
  return true;
}

/**
 * Function used to open the index for reading
 */
bool GifIndex::open() {
  if (_file) _file.close();
  _count = 0;
  _position = 0;
  _recent_size = 0;
  IndexSignature signature;
  if (!readSignature(signature) || !_file.open(INDEX_PATH, O_RDONLY)) {
    Serial.println("SD: No GIF index available");
    return false;
  }
  // The first record is the header
  _count = _file.size() / INDEX_RECORD_LENGTH - 1;
  return true;
}

uint32_t GifIndex::count() const { return _count; }

/**
 * Function used to get the next GIF of the index
 *
 * @return false if the index is empty or all GIFs were played in order
 */
bool GifIndex::next(String& path) {
  if (_count == 0) return false;
  if (!play_random) {
    if (_position >= _count) return false;
    return read(_position++, path);
  }

  // Avoid repeating the recently played GIFs, at most half of the GIFs are
  // remembered so there are always enough left to pick from
  uint32_t record = esp_random() % _count;
  while (recentlyPlayed(record)) record = esp_random() % _count;
  remember(record);
  return read(record, path);
}

void GifIndex::rewind() { _position = 0; }

bool GifIndex::read(uint32_t record, String& path) {
  char buffer[INDEX_RECORD_LENGTH];
  // Skip the header
  uint64_t offset = (static_cast<uint64_t>(record) + 1) * INDEX_RECORD_LENGTH;
  if (!_file.seekSet(offset) ||
      _file.read(buffer, sizeof(buffer)) != sizeof(buffer)) {
    Serial.println("SD: Failed to read the index");
    return false;
  }
  buffer[sizeof(buffer) - 1] = '\0';
  path = buffer;
  return true;
}

bool GifIndex::recentlyPlayed(uint32_t record) const {
  for (size_t i = 0; i < _recent_size; i++) {
    if (_recent[i] == record) return true;
  }
  return false;
}

void GifIndex::remember(uint32_t record) {
  size_t capacity = std::min<size_t>(RECENT_COUNT, _count / 2);
  if (capacity == 0) return;
  _recent[_recent_next % capacity] = record;
  _recent_next = (_recent_next + 1) % capacity;
  _recent_size = std::min(_recent_size + 1, capacity);
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
