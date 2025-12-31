# GitHub Copilot Instructions for 7-Segment LED Clock

## Project Overview

This is an ESP32-based 7-segment RGB LED clock using PlatformIO IDE. The clock displays time synced via NTP, shows temperature from OpenWeatherMap API, and supports WiFi Manager configuration with double-reset detection.

## Development Guidelines

### Arduino/PlatformIO Best Practices

- Use PlatformIO framework for ESP32 (espressif32 platform)
- **IMPORTANT**: Always activate PlatformIO virtual environment before running commands:
  ```bash
  source ~/.platformio/penv/bin/activate && platformio run
  ```
- Follow Arduino core API conventions for ESP32
- Use proper header guards in all `.h` files
- Implement proper error handling for network operations (WiFi, HTTP, NTP)
- Use `const` for constants and read-only data
- Prefer `constexpr` for compile-time constants
- Initialize all variables at declaration when possible

### C++ Coding Standards

- Use C++11 or newer features supported by ESP32 Arduino core
- Prefer `nullptr` over `NULL`
- Use smart pointers when appropriate (though limited in embedded context)
- Avoid dynamic memory allocation where possible to prevent fragmentation
- Use `const char*` or `std::string` instead of Arduino `String` for better memory management
- Follow RAII principles for resource management

### Code Formatting

- Use single blank line to separate functions, classes, and code blocks
- NO white spaces at the end of lines
- Use 2 or 4 space indentation consistently (check existing code)
- Place opening braces on the same line for functions and control structures
- Use descriptive variable and function names (camelCase for variables, PascalCase for classes)

### Hardware-Specific Guidelines

- All hardware pins and configurations must be defined in `include/config.h`
- Use FastLED library for WS2812 RGB LED control
- Respect ESP32 memory constraints (avoid large stack allocations)
- Use PROGMEM for large constant data structures when appropriate
- Handle WiFi connection failures gracefully with timeouts and retries

### Library Usage

- All libraries managed through `platformio.ini` `lib_deps`
- Use TaskScheduler for periodic task execution
- Use ArduinoJson v6 for JSON parsing (OpenWeatherMap API)
- Use ESPAsync_WiFiManager for WiFi configuration portal
- Use ESP_DoubleResetDetector for config portal trigger

### Configuration Management

- NEVER modify `_legacy/` folder contents
- All user-configurable settings must be in `include/config.h`
- Keep `include/config_rename.h` as template synchronized with `config.h` structure
- Use `#define` for compile-time constants
- Use typed variables for runtime-configurable values

### File Organization

- Header files in `include/` directory
- Implementation in `src/` directory
- Keep `src/main.cpp` focused on setup() and loop()
- Separate concerns into logical modules (WiFi, LED display, weather, etc.)

### Error Handling

- Check return values from WiFi, HTTP, and NTP operations
- Display error messages on LED display when appropriate ("r", "ConF", etc.)
- Log errors to Serial when DEBUG is enabled
- Implement timeouts for all network operations

### Testing Considerations

- Code should compile without warnings
- Test WiFi connection with both saved credentials and config portal
- Verify NTP time synchronization
- Test OpenWeatherMap API calls and JSON parsing
- Validate LED display for all characters and modes (SOLID, PALETTE)
- Test double-reset detection within DRD_TIMEOUT window

### Future Compatibility

- Code structure should support OTA (Over-The-Air) updates
- Keep WiFi credentials and API keys in config.h (gitignored)
- Design modular architecture for easy feature additions

## Important Reminders

- **NEVER** touch the `_legacy/` folder
- Always preserve all configuration options from legacy code
- Follow the "one blank line separation" rule strictly
- Remove trailing whitespace from all lines
- This copyright notices must be preserved in all files containing own code (except `include/config.h` or any 3rd party libraries).
  Update or add it as needed to match the following text exactly (set current year appropriately):
  ```plain
  /*
   * This file is part of the 7 Segment LED Clock Project
   *   https://github.com/ursweiss/7-Segment-LED-Clock
   *   https://www.printables.com/model/68013-7-segment-led-clock
   *
   * Copyright (c) 2021-{{current_year}} Urs Weiss
   *
   * This program is free software: you can redistribute it and/or modify
   * it under the terms of the GNU General Public License as published by
   * the Free Software Foundation, version 3.
   *
   * This program is distributed in the hope that it will be useful, but
   * WITHOUT ANY WARRANTY; without even the implied warranty of
   * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   * General Public License for more details.
   *
   * You should have received a copy of the GNU General Public License
   * along with this program. If not, see <http://www.gnu.org/licenses/>.
   */
  ```
