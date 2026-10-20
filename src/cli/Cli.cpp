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

#include "Cli.hpp"

#include <WiFi.h>

#include "storage/Configuration.hpp"
#include "update/Version.hpp"

namespace cli {

using storage::settings::Setting;

/**
 * Constructor of the Cli class
 *
 * @param on_change Invoked after a setting was changed, used to apply
 *                  settings that take effect immediately
 * @param actions Actions added to the menus
 */
Cli::Cli(ChangeHandler on_change, std::vector<Action> actions)
    : _on_change(std::move(on_change)), _actions(std::move(actions)) {}

void Cli::begin() {
  Serial.println("CLI: Press Enter to open the configuration menu");
}

/**
 * Function used to read the serial input, characters are echoed as most
 * serial monitors do not echo the input themselves
 */
void Cli::loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();

    // Treat \r, \n and \r\n as a single line ending
    if (c == '\n' && _last_was_cr) {
      _last_was_cr = false;
      continue;
    }
    _last_was_cr = c == '\r';
    if (c == '\r' || c == '\n') {
      Serial.println();
      std::string line = _line;
      _line.clear();
      handleLine(line);
      continue;
    }

    if (c == '\b' || c == 0x7f) {
      if (!_line.empty()) {
        _line.pop_back();
        Serial.print("\b \b");
      }
    } else if (isprint(static_cast<unsigned char>(c)) &&
               _line.length() < MAX_LINE_LENGTH) {
      _line += c;
      Serial.print(c);
    }
  }
}

void Cli::handleLine(const std::string& line) {
  // Any input opens the menu
  if (!_active) {
    _active = true;
    _path.clear();
    printMenu();
    printPrompt();
    return;
  }

  // Trim surrounding whitespace
  size_t start = line.find_first_not_of(' ');
  size_t end = line.find_last_not_of(' ');
  std::string input =
      start == std::string::npos ? "" : line.substr(start, end - start + 1);

  if (_editing) {
    handleSettingInput(input);
  } else if (_confirming || _confirm_reboot) {
    handleConfirmation(input);
  } else {
    handleMenuInput(input);
  }
  if (_active) printPrompt();
}

void Cli::handleMenuInput(const std::string& input) {
  if (input.empty() || input == "h" || input == "?") {
    printMenu();
  } else if (input == "b" || input == "..") {
    back();
  } else if (input == "s") {
    save();
  } else if (input == "r") {
    reboot();
  } else if (input == "i") {
    printInfo();
  } else if (input == "q") {
    if (_unsaved) {
      Serial.println(
          "Note: There are unsaved changes, they are kept until "
          "the next reboot");
    }
    Serial.println("Closed the menu, press Enter to open it again");
    _active = false;
  } else {
    char* end = nullptr;
    long index = strtol(input.c_str(), &end, 10);
    std::vector<Entry> menu = entries();
    if (*end != '\0' || index < 1 || index > static_cast<long>(menu.size())) {
      Serial.println("Unknown input, enter h for help");
      return;
    }
    select(menu[index - 1]);
  }
}

void Cli::select(const Entry& entry) {
  if (entry.isMenu()) {
    _path = _path.empty() ? entry.name : _path + "/" + entry.name;
    printMenu();
  } else if (entry.setting) {
    _editing = entry.setting;
    Serial.printf("%s\r\n", _editing->description);
    Serial.printf("Current value: %s, allowed: %s\r\n",
                  _editing->toString().c_str(), _editing->range().c_str());
    Serial.println("Enter the new value, an empty line keeps the current one");
  } else if (entry.action->confirm) {
    _confirming = entry.action;
    Serial.printf("%s, enter yes to continue\r\n", entry.action->description);
  } else {
    entry.action->run();
  }
}

void Cli::handleSettingInput(const std::string& input) {
  Setting* setting = _editing;
  _editing = nullptr;
  if (input.empty()) {
    Serial.println("Unchanged");
    return;
  }

  std::string error;
  if (!setting->parse(input, error)) {
    Serial.printf("Invalid value: %s\r\n", error.c_str());
    return;
  }
  _unsaved = true;
  Serial.printf("%s = %s\r\n", setting->name(), setting->toString().c_str());
  if (setting->hasFlag(storage::settings::REBOOT)) {
    _reboot_required = true;
    Serial.println("Takes effect after saving (s) and rebooting (r)");
  } else {
    Serial.println("Applied, save (s) to keep it after a reboot");
  }
  if (_on_change) _on_change(*setting);
}

void Cli::handleConfirmation(const std::string& input) {
  bool confirmed = input == "yes";
  const Action* action = _confirming;
  bool reboot = _confirm_reboot;
  _confirming = nullptr;
  _confirm_reboot = false;

  if (!confirmed) {
    Serial.println("Cancelled");
  } else if (action) {
    action->run();
  } else if (reboot) {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }
}

void Cli::back() {
  size_t separator = _path.rfind('/');
  _path = separator == std::string::npos ? "" : _path.substr(0, separator);
  printMenu();
}

void Cli::save() {
  if (storage::configuration::Configuration::save()) {
    _unsaved = false;
    Serial.println(_reboot_required ? "Saved, reboot (r) to apply all changes"
                                    : "Saved");
  } else {
    Serial.println("Saving failed");
  }
}

void Cli::reboot() {
  if (_unsaved) {
    Serial.println(
        "There are unsaved changes which will be lost, enter yes "
        "to reboot anyway");
  } else {
    Serial.println("Enter yes to reboot");
  }
  _confirm_reboot = true;
}

void Cli::printInfo() {
  Serial.printf("Firmware:  %s (%s)\r\n", ESPINDMD_VERSION, ESPINDMD_ENV);
  Serial.printf("MAC:       %s\r\n", WiFi.macAddress().c_str());
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Wi-Fi:     %s (%d dBm)\r\n", WiFi.SSID().c_str(),
                  WiFi.RSSI());
    Serial.printf("IP:        %s\r\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("Wi-Fi:     Not connected");
  }
  Serial.printf("Free heap: %lu bytes\r\n",
                static_cast<unsigned long>(ESP.getFreeHeap()));
  Serial.printf("Uptime:    %lu s\r\n",
                static_cast<unsigned long>(millis() / 1000));
}

/**
 * Function used to collect the entries of the current menu. Every setting
 * and action below the current path either becomes a direct entry or adds
 * a sub menu, in the order they are defined.
 */
std::vector<Cli::Entry> Cli::entries() const {
  std::string prefix = _path.empty() ? "" : _path + "/";
  std::vector<Entry> result;

  auto add = [&](const char* path, Setting* setting, const Action* action) {
    std::string full = path;
    if (full.compare(0, prefix.length(), prefix) != 0) return;
    std::string rest = full.substr(prefix.length());
    size_t separator = rest.find('/');
    if (separator == std::string::npos) {
      Entry entry;
      entry.name = rest;
      entry.setting = setting;
      entry.action = action;
      result.push_back(entry);
      return;
    }
    std::string menu = rest.substr(0, separator);
    for (const Entry& existing : result) {
      if (existing.isMenu() && existing.name == menu) return;
    }
    Entry entry;
    entry.name = menu;
    result.push_back(entry);
  };

  for (size_t i = 0; i < storage::settings::count; i++) {
    Setting& setting = storage::settings::all[i];
    if (setting.hasFlag(storage::settings::HIDDEN)) continue;
    add(setting.path, &setting, nullptr);
  }
  for (const Action& action : _actions) add(action.path, nullptr, &action);
  return result;
}

void Cli::printMenu() {
  Serial.printf("\r\n== esPINDMD configuration: /%s ==\r\n", _path.c_str());
  std::vector<Entry> menu = entries();
  for (size_t i = 0; i < menu.size(); i++) {
    const Entry& entry = menu[i];
    std::string name = entry.isMenu() ? entry.name + "/" : entry.name;
    std::string value;
    if (entry.setting) {
      value = entry.setting->toString();
    } else if (entry.action) {
      value = std::string("[") + entry.action->description + "]";
    }
    if (name.length() < NAME_WIDTH)
      name.append(NAME_WIDTH - name.length(), ' ');
    Serial.printf("  %2u) %s %s\r\n", static_cast<unsigned>(i + 1),
                  name.c_str(), value.c_str());
  }
  Serial.println();
  Serial.printf("  %s s) save  r) reboot  i) info  h) help  q) quit\r\n",
                _path.empty() ? "" : "b) back ");
}

void Cli::printPrompt() {
  if (_editing) {
    Serial.printf("%s> ", _editing->name());
  } else {
    // Unsaved changes are marked with a *
    Serial.printf("esPINDMD:/%s%s> ", _path.c_str(), _unsaved ? "*" : "");
  }
}

}  // namespace cli
