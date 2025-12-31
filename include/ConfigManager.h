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
#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h>
#include <FastLED.h>

// Configuration structure matching config.h settings
struct Config {
  // WiFi
  String portalSsid;
  String portalPassword;

  // Clock
  uint8_t clockColorMode;
  CRGB clockColorSolid;
  uint8_t clockColorPaletteIndex;  // Store palette as index (0=Rainbow, 1=Cloud, 2=Lava, 3=Ocean, 4=Forest)
  uint8_t clockColorCharBlend;
  uint8_t clockColorBlending;  // 0=NOBLEND, 1=LINEARBLEND
  uint8_t clockSecIndicatorDiff;

  // Weather
  String locationLatitude;
  String locationLongitude;
  String locationUnits;
  uint8_t weatherTempEnabled;
  uint8_t weatherTempDisplayTime;
  int8_t weatherTempMin;
  int8_t weatherTempMax;
  String weatherTempSchedule;
  String weatherUpdateSchedule;

  // FastLED
  uint8_t ledBrightness;
  uint8_t ledDimEnabled;
  uint8_t ledDimBrightness;
  uint8_t ledDimFadeDuration;
  String ledDimStartTime;
  String ledDimEndTime;

  // Clock Update
  String clockUpdateSchedule;
};

// ConfigManager class for handling configuration persistence
class ConfigManager {
public:
  ConfigManager();

  // Initialize LittleFS and load configuration
  bool begin();

  // Load configuration from LittleFS (/config.json)
  bool loadConfig();

  // Save configuration to LittleFS (/config.json)
  bool saveConfig();

  // Reset configuration to defaults from config.h
  void resetConfig();

  // Get current configuration
  Config& getConfig();

  // Get schema JSON from LittleFS
  String getSchema();

  // Validate configuration values are within acceptable ranges
  bool validateConfig();

  // Helper to convert palette to/from index
  static CRGBPalette16 getPaletteByIndex(uint8_t index);

private:
  Config config;
  bool initialized;

  // Load defaults from config.h
  void loadDefaults();
};

// Global instance
extern ConfigManager configManager;

#endif // CONFIGMANAGER_H
