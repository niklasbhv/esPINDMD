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

#include <SdFat.h>

class SequentialIterator {
 private:
    SdFat &_sd;
  SdFile _dir;
  bool _isOpen;
  SequentialIterator *_child;

  // Helper: get path of current directory (if possible)
  const char *_dirName() {
    static char name[64];
    _dir.getName(name, sizeof(name));
    return name;
  }
 public:
  SequentialIterator(SdFat &sd, const char *path)
      : _sd(sd), _isOpen(false), _child(nullptr) {
    if (_dir.open(path)) {
      _isOpen = true;
    } else {
      Serial.print("Failed to open directory: ");
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
  bool next(char *fileName, size_t nameSize);
};

class IndexedIterator {
 private:
 public:
  void next();
};