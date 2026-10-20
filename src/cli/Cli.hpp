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

#include <Arduino.h>

#include <functional>
#include <string>
#include <vector>

#include "storage/Settings.hpp"

namespace cli {

// Width of the name column in menus
constexpr size_t NAME_WIDTH = 24;
// Maximum length of an input line
constexpr size_t MAX_LINE_LENGTH = 255;

/**
 * An action that can be executed from the menu, placed in the menu tree
 * by its path just like a setting, e.g. "update/install"
 */
struct Action {
  const char* path;
  const char* description;
  std::function<void()> run;
  // Ask for confirmation before running the action
  bool confirm;
};

using ChangeHandler = std::function<void(const storage::settings::Setting&)>;

/**
 * Menu based command line interface on the serial console. The menus
 * follow the structure of the configuration file, settings and actions
 * are selected by their number.
 *
 * The CLI is non-blocking, loop() needs to be called frequently.
 */
class Cli {
 private:
  /**
   * An entry of the current menu, either a sub menu, a setting or an action
   */
  struct Entry {
    std::string name;
    storage::settings::Setting* setting = nullptr;
    const Action* action = nullptr;
    bool isMenu() const { return setting == nullptr && action == nullptr; }
  };

  ChangeHandler _on_change;
  std::vector<Action> _actions;

  std::string _line;
  bool _last_was_cr = false;
  bool _active = false;
  // Current menu, empty for the root menu
  std::string _path;
  // Pending input for a setting or a confirmation of an action
  storage::settings::Setting* _editing = nullptr;
  const Action* _confirming = nullptr;
  bool _confirm_reboot = false;
  bool _unsaved = false;
  bool _reboot_required = false;

  std::vector<Entry> entries() const;
  void handleLine(const std::string& line);
  void handleMenuInput(const std::string& input);
  void handleSettingInput(const std::string& input);
  void handleConfirmation(const std::string& input);
  void select(const Entry& entry);
  void back();
  void save();
  void reboot();
  void printInfo();
  void printMenu();
  void printPrompt();

 public:
  Cli(ChangeHandler on_change, std::vector<Action> actions);
  void begin();
  void loop();
};

}  // namespace cli
