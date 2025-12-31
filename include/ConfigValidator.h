/*
 * This file is part of the 7 Segment LED Clock Project
 *   https://github.com/ursweiss/7-Segment-LED-Clock
 *   https://www.printables.com/model/68013-7-segment-led-clock
 *
 * Copyright (c) 2021-2025 Urs Weiss
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

#ifndef CONFIG_VALIDATOR_H
#define CONFIG_VALIDATOR_H

#include <Arduino.h>

// Forward declaration
struct Config;

/**
 * Centralized validation logic for configuration values
 * Eliminates duplication between ConfigManager and WebConfig
 */
class ConfigValidator {
public:
  struct ValidationResult {
    bool isValid;
    String errorMessage;
    ValidationResult(bool valid = true, const String& error = "")
      : isValid(valid), errorMessage(error) {}
  };

  // Brightness validation (0-255)
  static ValidationResult validateBrightness(uint8_t brightness);

  // Color mode validation (0=solid, 1=palette)
  static ValidationResult validateColorMode(uint8_t mode);

  // Palette index validation (0-15)
  static ValidationResult validatePaletteIndex(uint8_t index);

  // Temperature mode validation (0=disabled, 1=once per hour)
  static ValidationResult validateTemperatureMode(uint8_t mode);

  // Time format validation (0=12h, 1=24h)
  static ValidationResult validateTimeFormat(uint8_t format);

  // Time string validation (HH:MM format)
  static ValidationResult validateTimeString(const String& timeStr);

  // WiFi credentials validation
  static ValidationResult validateSSID(const String& ssid);
  static ValidationResult validatePassword(const String& password);

  // Apply validation and auto-correct config
  static void validateAndCorrectConfig(Config& config);
};

#endif // CONFIG_VALIDATOR_H
