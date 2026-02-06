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

#define SD_CONFIG SdSpiConfig(SD_CS, DEDICATED_SPI, SPI_CLOCK)

Sd::Sd() {
  Serial.println("SD: Initializing the sd component...");
  Serial.println("\nSD SPI pins:\n");
  Serial.println("MISO: " + String(SD_MISO));
  Serial.println("MOSI: " + String(SD_MOSI));
  Serial.println("CLK:  " + String(SD_CLK));
  Serial.println("CS:   " + String(SD_CS));

  // try to initialize the sd card
  if (!sd.begin(SD_CONFIG)) {
    Serial.println("SD: Initialization of the sd component failed!");
    sd.initErrorHalt();
  }
  _sequentialIterator = std::make_unique<SequentialIterator>(sd, GIF_ROOT_PATH);
  Serial.println("SD: Initialized the sd component!");
}

// Function to check if a filename ends with .gif
bool Sd::isGifFile(const char *filename) {
  const char *ext = strrchr(filename, '.');
  return (ext && strcasecmp(ext, ".gif") == 0);
}

// Recursive function to scan directories
void Sd::indexGifFiles(SdFile dir, String pathPrefix) {  // = ""
  SdFile entry;

  while (entry.openNext(&dir, O_RDONLY)) {
    if (entry.isDir()) {
      char subDirName[64];
      entry.getName(subDirName, sizeof(subDirName));

      // Recurse into the subdirectory
      // indexGifFiles(entry, pathPrefix + "/" + subDirName);
    } else {
      char fileName[64];
      entry.getName(fileName, sizeof(fileName));
      if (isGifFile(fileName)) {
        String fullPath = pathPrefix + "/" + fileName;
        // indexFile.println(fullPath);
      }
    }
    entry.close();
  }
}

/**
 * Pre-processing a index to enable randomized playback of gifs
 */
int Sd::generateFileIndex(const char *folderPath, const char *indexFilename) {
  if (!sd.begin()) {
    Serial.println("SD init failed!");
    return false;
  }

  SdFile root;
  if (!root.open(GIF_ROOT_PATH)) {
    Serial.println("Failed to open root folder!");
    return false;
  }

  // if (!indexFile.open(INDEX_FILENAME, O_WRITE | O_CREAT | O_TRUNC)) {
  //   Serial.println("Failed to create index file!");
  //   return false;
  // }

  // indexGifFiles(root, "");  // Start recursive indexing
  // indexFile.close();
  Serial.println("GIF file index created successfully!");
  return true;

  // indexFile.close();
  // dir.close();

  Serial.println("Indexing complete.");
  return 0;
}

/**
 * Function used to load a index file
 */
int Sd::loadFileIndex(const char *indexFilename) {
  FsFile indexFile;
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

bool Sd::openFile(String &filename, FsFile &file) {
  return file.open(filename.c_str(), O_READ);
}

bool Sd::openFile(const char *filename, FsFile &file) {
  return file.open(filename, O_READ);
}

bool Sd::closeFile(FsFile &file) { return file.close(); }

bool SequentialIterator::next(String &filename) {
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

  FsFile entry;

  while (entry.openNext(&_dir, O_RDONLY)) {
    // Check if the entry is a file
    if (entry.isFile()) {
      char filename_buffer[MAX_FILENAME_LENGTH];
      size_t filename_size =
          entry.getName(filename_buffer, sizeof(filename_buffer));
      filename = String(_path + "/" + String(filename_buffer, filename_size));
      entry.close();
      return true;
    }
    // Check if the entry is a directory
    else if (entry.isDir()) {
      char subDirName[MAX_DIRNAME_LENGTH];
      entry.getName(subDirName, sizeof(subDirName));

      // Create new iterator for subdir
      _child = new SequentialIterator(sd, String(_path + "/" + subDirName));
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

void Sd::resetIterator() {
  _sequentialIterator.reset();
  _sequentialIterator = std::make_unique<SequentialIterator>(sd, GIF_ROOT_PATH);
}

bool Sd::next(String &file_path) {
  file_path.clear();
  return _sequentialIterator->next(file_path);
}

void IndexedIterator::next() {}

void *Sd::openGifFile(const char *fname, int32_t *pSize) {
  file = sd.open(fname);
  if (file) {
    *pSize = file.size();
    return (void *)&file;
  }
  return NULL;
}

void Sd::closeGifFile(void *pHandle) {
  FsFile *file = static_cast<FsFile *>(pHandle);
  if (file != NULL) file->close();
}

int32_t Sd::readGifFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
  int32_t iBytesRead;
  iBytesRead = iLen;
  FsFile *file = static_cast<FsFile *>(pFile->fHandle);
  // Note: If you read a file all the way to the last byte, seek() stops working
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos - 1;  // <-- ugly work-around
  if (iBytesRead <= 0) return 0;
  iBytesRead = (int32_t)file->read(pBuf, iBytesRead);
  pFile->iPos = file->position();
  return iBytesRead;
}

int32_t Sd::seekGifFile(GIFFILE *pFile, int32_t iPosition) {
  FsFile *file = static_cast<FsFile *>(pFile->fHandle);
  file->seek(iPosition);
  pFile->iPos = (int32_t)file->position();
  return pFile->iPos;
}