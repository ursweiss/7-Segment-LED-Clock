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

#include "ConfigValidator.h"
#include "ConfigManager.h"
#include "Logger.h"
#include <ctype.h>

ConfigValidator::ValidationResult ConfigValidator::validateBrightness(uint8_t brightness) {
  // All uint8_t values are valid (0-255)
  return ValidationResult(true);
}

ConfigValidator::ValidationResult ConfigValidator::validateColorMode(uint8_t mode) {
  if (mode > 1) {
    return ValidationResult(false, "Color mode must be 0 (solid) or 1 (palette)");
  }
  return ValidationResult(true);
}

ConfigValidator::ValidationResult ConfigValidator::validatePaletteIndex(uint8_t index) {
  // Palette index validation removed - schema uses different field name (clockColorPaletteIndex)
  // This function left for future compatibility
  return ValidationResult(true);
}

ConfigValidator::ValidationResult ConfigValidator::validateTemperatureMode(uint8_t mode) {
  if (mode > 1) {
    return ValidationResult(false, "Temperature mode must be 0 (disabled) or 1 (once per hour)");
  }
  return ValidationResult(true);
}

ConfigValidator::ValidationResult ConfigValidator::validateTimeFormat(uint8_t format) {
  if (format > 1) {
    return ValidationResult(false, "Time format must be 0 (12h) or 1 (24h)");
  }
  return ValidationResult(true);
}

ConfigValidator::ValidationResult ConfigValidator::validateTimeString(const String& timeStr) {
  if (timeStr.length() != 5) {
    return ValidationResult(false, "Time must be in HH:MM format");
  }
  if (timeStr.charAt(2) != ':') {
    return ValidationResult(false, "Time must use ':' separator");
  }
  if (!isdigit(timeStr.charAt(0)) || !isdigit(timeStr.charAt(1)) ||
      !isdigit(timeStr.charAt(3)) || !isdigit(timeStr.charAt(4))) {
    return ValidationResult(false, "Time must contain only digits and ':'");
  }

  int hours = timeStr.substring(0, 2).toInt();
  int minutes = timeStr.substring(3, 5).toInt();

  if (hours > 23) {
    return ValidationResult(false, "Hours must be 00-23");
  }
  if (minutes > 59) {
    return ValidationResult(false, "Minutes must be 00-59");
  }

  return ValidationResult(true);
}

ConfigValidator::ValidationResult ConfigValidator::validateSSID(const String& ssid) {
  if (ssid.length() == 0) {
    return ValidationResult(false, "SSID cannot be empty");
  }
  if (ssid.length() > 32) {
    return ValidationResult(false, "SSID cannot exceed 32 characters");
  }
  return ValidationResult(true);
}

ConfigValidator::ValidationResult ConfigValidator::validatePassword(const String& password) {
  if (password.length() > 0 && password.length() < 8) {
    return ValidationResult(false, "Password must be at least 8 characters or empty");
  }
  if (password.length() > 63) {
    return ValidationResult(false, "Password cannot exceed 63 characters");
  }
  return ValidationResult(true);
}

void ConfigValidator::validateAndCorrectConfig(Config& config) {
  // Brightness - all uint8_t values valid (0-255)

  // Color mode
  auto colorModeResult = validateColorMode(config.clockColorMode);
  if (!colorModeResult.isValid) {
    LOG_WARNF("Invalid color mode %d, resetting to 0 (solid)", config.clockColorMode);
    config.clockColorMode = 0;
  }

  // Dim times
  auto dimStartResult = validateTimeString(config.ledDimStartTime);
  if (!dimStartResult.isValid) {
    LOG_WARNF("Invalid dim start time '%s', resetting to '22:00'", config.ledDimStartTime.c_str());
    config.ledDimStartTime = "22:00";
  }

  auto dimEndResult = validateTimeString(config.ledDimEndTime);
  if (!dimEndResult.isValid) {
    LOG_WARNF("Invalid dim end time '%s', resetting to '07:00'", config.ledDimEndTime.c_str());
    config.ledDimEndTime = "07:00";
  }

  LOG_INFO("Config validation complete");
}
