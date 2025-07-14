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

#include "GifLoader.hpp"

bool SequentialIterator::next(char *fileName, size_t nameSize) {
    // If we are in a subdirectory, check that first
    if (_child) {
        if (_child->next(fileName, nameSize)) {
        return true; // Still files in subdir
        } else {
        delete _child;
        _child = nullptr; // Subdir done, continue parent dir
        }
    }

    if (!_isOpen) return false;

    SdFile entry;

    while (entry.openNext(&_dir, O_RDONLY)) {
        if (entry.isFile()) {
        entry.getName(fileName, nameSize);
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
        if (_child->next(fileName, nameSize)) {
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

void IndexedIterator::next() {}