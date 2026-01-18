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

#include <Arduino.h>

#include "controller/SystemController.hpp"

// Global instance of the system controller
SystemController controller;

// Task handle
TaskHandle_t systemTask;

void sytemLoop(void *pvParameters) {
  while (true) {
    controller.systemLoop();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  // begin the serial console
  Serial.begin(115200);
  Serial.println("Setup: Initializing the system controller...");
  // initialize the SystemController
  controller.begin();
  Serial.println("Setup: Initialized the system controller!");

  // Create a seperate task for core system functionalities
  // xTaskCreate(sytemLoop, "SystemLoop", 4096, NULL, 1, &systemTask);
}

void loop() {
  controller.applicationLoop();
  controller.systemLoop();
}
