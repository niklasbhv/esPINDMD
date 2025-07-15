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

Sd::Sd() {
  Serial.println("SD: Initializing the sd component...");
  Serial.println("\nSPI pins:\n");
  Serial.println("MISO: " + String(SD_MISO));
  Serial.println("MOSI: " + String(SD_MOSI));
  Serial.println("CLK:  " + String(SD_CLK));
  Serial.println("CS:   " + String(SD_CS));

  SoftSpiDriver<SD_MISO, SD_MOSI, SD_CLK> softSpi;

  // try to initialize the sd card
  if (!_sd.begin(SD_CS, SPI_SPEED)) {
    Serial.println("SD: Initialization of the sd component failed!");
  }
  _sequentialIterator = std::make_unique<SequentialIterator>(_sd, GIF_ROOT_PATH);
  Serial.println("SD: Initialized the sd component!");
}

/**
 * Pre-processing a index to enable randomized playback of gifs
 */
int Sd::generateFileIndex(const char* folderPath, const char* indexFilename) {
  SdFile dir, entry, indexFile;

  if (!dir.open(folderPath)) {
    Serial.println("Failed to open folder");
    return -1;
  }

  if (!indexFile.open(indexFilename, O_WRITE | O_CREAT | O_TRUNC)) {
    Serial.println("Failed to create index file");
    return -1;
  }

  while (entry.openNext(&dir, O_RDONLY)) {
    if (!entry.isDir()) {
      char name[MAX_FILENAME_LENGTH];
      entry.getName(name, sizeof(name));
      String fname(name);
      // fname.toLowerCase();
      if (fname.endsWith(".gif")) {
        indexFile.println(fname.c_str());
        Serial.println("Indexed: " + fname);
      }
    }
    entry.close();
  }

  indexFile.close();
  dir.close();

  Serial.println("Indexing complete.");
  return 0;
}

/**
 * Function used to load a index file
 */
int Sd::loadFileIndex(const char* indexFilename) {
  SdFile indexFile;
  if (!indexFile.open(indexFilename, O_READ)) {
    Serial.println("Failed to open index file");
    return -1;
  }

  char line[MAX_FILENAME_LENGTH];
  while (indexFile.fgets(line, sizeof(line)) && _gifCount < MAX_GIF_FILES) {
    _gifFiles[_gifCount++] = String(line);  // .trim() Remove newline
  }

  indexFile.close();

  Serial.print("Loaded ");
  Serial.print(_gifCount);
  Serial.println(" files from index.");

  return 0;
}

bool Sd::openFile(String& filename, FsFile& file) {
  return file.open(filename.c_str(), O_READ);
}

bool Sd::closeFile(FsFile& file) { return file.close(); }

bool SequentialIterator::next(String filename) {
  // If we are in a subdirectory, check that first
  if (_child) {
    if (_child->next(filename)) {
      return true;  // Still files in subdir
    } else {
      delete _child;
      _child = nullptr;  // Subdir done, continue parent dir
    }
  }

  if (!_isOpen) return false;

  SdFile entry;

  while (entry.openNext(&_dir, O_RDONLY)) {
    if (entry.isFile()) {
      char* filename_buffer;
      size_t filename_buffer_size;
      entry.getName(filename_buffer, filename_buffer_size);
      std::string filename(filename_buffer, filename_buffer_size);
      entry.close();
      return true;
    } else if (entry.isDir()) {
      char subDirName[64];
      entry.getName(subDirName, sizeof(subDirName));
      Serial.print("Entering subdirectory: ");
      Serial.println(subDirName);

      // Build full subdir path
      char subDirPath[128];
      snprintf(subDirPath, sizeof(subDirPath), "%s/%s", _dirName(), subDirName);

      // Create new iterator for subdir
      _child = new SequentialIterator(_sd, subDirPath);

      entry.close();

      // Immediately get next file from subdir
      if (_child->next(filename)) {
        return true;
      } else {
        delete _child;
        _child = nullptr;
      }
    }
    entry.close();
  }

  // Done with this directory
  _dir.close();
  _isOpen = false;
  return false;
}

bool Sd::next(String filename) {
  return _sequentialIterator->next(filename);
}

void IndexedIterator::next() {}