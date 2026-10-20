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

// Both values are injected by scripts/version.py, the fallbacks are only
// used if the sources are built without it

// Firmware version, e.g. 0.1.0
#ifndef ESPINDMD_VERSION
#define ESPINDMD_VERSION "0.0.0-dev"
#endif

// PlatformIO environment the firmware was built for, e.g. esp32dev
#ifndef ESPINDMD_ENV
#define ESPINDMD_ENV "unknown"
#endif
