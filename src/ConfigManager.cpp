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

#include "ConfigManager.h"
#include "config.h"
#include "Logger.h"
#include "schema.h"
#include "LED_Clock.h"
#include "BrightnessControl.h"
#include "CronHelper.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>

// Global instance
ConfigManager configManager;

ConfigManager::ConfigManager() : initialized(false) {
}

bool ConfigManager::begin() {
  if (initialized) {
    return true;
  }

  LOG_INFO("Initializing LittleFS...");

  if (!LittleFS.begin(true)) {  // true = format on failure
    LOG_ERROR("Failed to mount LittleFS");
    return false;
  }

  LOG_INFO("LittleFS mounted successfully");

  // Try to load existing config, or use defaults
  if (!loadConfig()) {
    LOG_WARN("No config found, using defaults");
    loadDefaults();
    saveConfig();  // Save defaults for next boot
  }

  initialized = true;
  return true;
}

void ConfigManager::loadDefaults() {
  LOG_INFO("Loading default configuration...");

  // WiFi
  config.portalSsid = "LED-Clock-Config";
  config.portalPassword = portalPassword;

  // Clock
  config.clockColorMode = clockColorMode;
  config.clockColorSolid = clockColorSolid;
  config.clockColorPaletteIndex = 0; // Default to Rainbow
  config.clockColorCharBlend = clockColorCharBlend;
  config.clockColorBlending = (clockColorBlending == LINEARBLEND) ? 1 : 0;
  config.clockSecIndicatorDiff = clockSecIndicatorDiff;

  // Weather
  config.locationLatitude = locationLatitude;
  config.locationLongitude = locationLongitude;
  config.locationUnits = locationUnits;
  config.weatherTempEnabled = weatherTempEnabled;
  config.weatherTempDisplayTime = weatherTempDisplayTime;
  config.weatherTempMin = weatherTempMin;
  config.weatherTempMax = weatherTempMax;
  config.weatherTempSchedule = weatherTempSchedule;
  config.weatherUpdateSchedule = weatherUpdateSchedule;

  // FastLED
  config.ledBrightness = ledBrightness;
  config.ledDimEnabled = ledDimEnabled;
  config.ledDimBrightness = ledDimBrightness;
  config.ledDimFadeDuration = ledDimFadeDuration;
  config.ledDimStartTime = ledDimStartTime;
  config.ledDimEndTime = ledDimEndTime;

  // Clock Update
  config.clockUpdateSchedule = clockUpdateSchedule;
}

bool ConfigManager::loadConfig() {
  if (!LittleFS.exists("/config.json")) {
    LOG_WARN("Config file does not exist");
    return false;
  }

  // Feed watchdog before filesystem operation
  esp_task_wdt_reset();

  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    LOG_ERROR("Failed to open config file for reading");

    // Check if filesystem is corrupted
    LOG_WARN("Attempting filesystem check...");
    if (!LittleFS.begin(false)) {  // Don't auto-format
      LOG_ERROR("LittleFS filesystem corrupted - formatting...");
      LittleFS.format();
      LittleFS.begin(true);
    }
    return false;
  }

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  // Feed watchdog after filesystem operation
  esp_task_wdt_reset();

  if (error) {
    LOG_ERROR("Failed to parse config JSON");
    return false;
  }

  LOG_INFO("Loading configuration from LittleFS...");

  // WiFi
  config.portalSsid = doc["portalSsid"] | "LED-Clock-Config";
  config.portalPassword = doc["portalPassword"] | "ledclock";

  // Clock
  config.clockColorMode = doc["clockColorMode"] | 1;
  config.clockColorSolid = doc["clockColorSolid"] | 0x00FF00;  // Green
  config.clockColorPaletteIndex = doc["clockColorPaletteIndex"] | 0;
  config.clockColorCharBlend = doc["clockColorCharBlend"] | 5;
  config.clockColorBlending = doc["clockColorBlending"] | 1;
  config.clockSecIndicatorDiff = doc["clockSecIndicatorDiff"] | 32;

  // Weather
  config.locationLatitude = doc["locationLatitude"] | "";
  config.locationLongitude = doc["locationLongitude"] | "";
  config.locationUnits = doc["locationUnits"] | "metric";
  config.weatherTempEnabled = doc["weatherTempEnabled"] | 1;
  config.weatherTempDisplayTime = doc["weatherTempDisplayTime"] | 5;
  config.weatherTempMin = doc["weatherTempMin"] | -40;
  config.weatherTempMax = doc["weatherTempMax"] | 50;
  config.weatherTempSchedule = doc["weatherTempSchedule"] | "30 * * * * *";
  config.weatherUpdateSchedule = doc["weatherUpdateSchedule"] | "0 5 * * * *";

  // FastLED
  config.ledBrightness = doc["ledBrightness"] | 128;
  config.ledDimEnabled = doc["ledDimEnabled"] | 1;
  config.ledDimBrightness = doc["ledDimBrightness"] | 64;
  config.ledDimFadeDuration = doc["ledDimFadeDuration"] | 30;
  config.ledDimStartTime = doc["ledDimStartTime"] | "22:00";
  config.ledDimEndTime = doc["ledDimEndTime"] | "06:00";

  // Clock Update
  config.clockUpdateSchedule = doc["clockUpdateSchedule"] | "* * * * * *";

  LOG_INFO("Configuration loaded successfully");
  // Validate loaded configuration
  if (!validateConfig()) {
    LOG_ERROR("Configuration validation failed, using defaults");
    loadDefaults();
    return false;
  }
  return true;
}

bool ConfigManager::saveConfig() {
  LOG_INFO("Saving configuration to LittleFS...");

  // Feed watchdog before filesystem operation
  esp_task_wdt_reset();

  StaticJsonDocument<2048> doc;

  // WiFi
  doc["portalSsid"] = config.portalSsid;
  doc["portalPassword"] = config.portalPassword;

  // Clock
  doc["clockColorMode"] = config.clockColorMode;
  doc["clockColorSolid"] = (uint32_t)config.clockColorSolid;
  doc["clockColorPaletteIndex"] = config.clockColorPaletteIndex;
  doc["clockColorCharBlend"] = config.clockColorCharBlend;
  doc["clockColorBlending"] = config.clockColorBlending;
  doc["clockSecIndicatorDiff"] = config.clockSecIndicatorDiff;

  // Weather
  doc["locationLatitude"] = config.locationLatitude;
  doc["locationLongitude"] = config.locationLongitude;
  doc["locationUnits"] = config.locationUnits;
  doc["weatherTempEnabled"] = config.weatherTempEnabled;
  doc["weatherTempDisplayTime"] = config.weatherTempDisplayTime;
  doc["weatherTempMin"] = config.weatherTempMin;
  doc["weatherTempMax"] = config.weatherTempMax;
  doc["weatherTempSchedule"] = config.weatherTempSchedule;
  doc["weatherUpdateSchedule"] = config.weatherUpdateSchedule;

  // FastLED
  doc["ledBrightness"] = config.ledBrightness;
  doc["ledDimEnabled"] = config.ledDimEnabled;
  doc["ledDimBrightness"] = config.ledDimBrightness;
  doc["ledDimFadeDuration"] = config.ledDimFadeDuration;
  doc["ledDimStartTime"] = config.ledDimStartTime;
  doc["ledDimEndTime"] = config.ledDimEndTime;

  // Clock Update
  doc["clockUpdateSchedule"] = config.clockUpdateSchedule;

  File file = LittleFS.open("/config.json", "w");
  if (!file) {
    LOG_ERROR("Failed to open config file for writing");
    return false;
  }

  if (serializeJson(doc, file) == 0) {
    LOG_ERROR("Failed to write config JSON");
    file.close();
    return false;
  }

  file.close();

  // Feed watchdog after filesystem operation
  esp_task_wdt_reset();

  // Invalidate caches so new config values are picked up
  markPaletteForUpdate();
  invalidateBrightnessCache();
  CronHelper::invalidateCache();

  LOG_INFO("Configuration saved successfully");
  return true;
}

bool ConfigManager::validateConfig() {
  bool valid = true;

  // Validate brightness values (0-255)
  if (config.ledBrightness > 255) {
    LOG_WARNF("Invalid ledBrightness: %d, resetting to 128", config.ledBrightness);
    config.ledBrightness = 128;
    valid = false;
  }
  if (config.ledDimBrightness > 255) {
    LOG_WARNF("Invalid ledDimBrightness: %d, resetting to 32", config.ledDimBrightness);
    config.ledDimBrightness = 32;
    valid = false;
  }

  // Validate clock color mode (0-2: SOLID, PALETTE, RAINBOW)
  if (config.clockColorMode > 2) {
    LOG_WARNF("Invalid clockColorMode: %d, resetting to 1", config.clockColorMode);
    config.clockColorMode = 1;
    valid = false;
  }

  // Validate palette index (0-4)
  if (config.clockColorPaletteIndex > 4) {
    LOG_WARNF("Invalid clockColorPaletteIndex: %d, resetting to 0", config.clockColorPaletteIndex);
    config.clockColorPaletteIndex = 0;
    valid = false;
  }

  // Validate blending mode (0-1)
  if (config.clockColorBlending > 1) {
    LOG_WARNF("Invalid clockColorBlending: %d, resetting to 1", config.clockColorBlending);
    config.clockColorBlending = 1;
    valid = false;
  }

  // Validate boolean flags (0-1)
  if (config.ledDimEnabled > 1) {
    config.ledDimEnabled = 1;
    valid = false;
  }
  if (config.weatherTempEnabled > 1) {
    config.weatherTempEnabled = 1;
    valid = false;
  }

  // Validate temperature range (-99 to 99)
  if (config.weatherTempMin < -99 || config.weatherTempMin > 99) {
    LOG_WARNF("Invalid weatherTempMin: %d, resetting to -40", config.weatherTempMin);
    config.weatherTempMin = -40;
    valid = false;
  }
  if (config.weatherTempMax < -99 || config.weatherTempMax > 99) {
    LOG_WARNF("Invalid weatherTempMax: %d, resetting to 50", config.weatherTempMax);
    config.weatherTempMax = 50;
    valid = false;
  }
  if (config.weatherTempMin >= config.weatherTempMax) {
    LOG_WARN("weatherTempMin >= weatherTempMax, resetting to defaults");
    config.weatherTempMin = -40;
    config.weatherTempMax = 50;
    valid = false;
  }

  // Validate temperature display time (1-60 seconds)
  if (config.weatherTempDisplayTime < 1 || config.weatherTempDisplayTime > 60) {
    LOG_WARNF("Invalid weatherTempDisplayTime: %d, resetting to 5", config.weatherTempDisplayTime);
    config.weatherTempDisplayTime = 5;
    valid = false;
  }

  // Validate fade duration (1-60 seconds)
  if (config.ledDimFadeDuration < 1 || config.ledDimFadeDuration > 60) {
    LOG_WARNF("Invalid ledDimFadeDuration: %d, resetting to 10", config.ledDimFadeDuration);
    config.ledDimFadeDuration = 10;
    valid = false;
  }

  // Validate time format (HH:MM)
  if (config.ledDimStartTime.length() != 5 || config.ledDimStartTime.charAt(2) != ':') {
    LOG_WARNF("Invalid ledDimStartTime format: %s, resetting to 22:00", config.ledDimStartTime.c_str());
    config.ledDimStartTime = "22:00";
    valid = false;
  }
  if (config.ledDimEndTime.length() != 5 || config.ledDimEndTime.charAt(2) != ':') {
    LOG_WARNF("Invalid ledDimEndTime format: %s, resetting to 07:00", config.ledDimEndTime.c_str());
    config.ledDimEndTime = "07:00";
    valid = false;
  }

  // Validate units (metric/imperial)
  if (config.locationUnits != "metric" && config.locationUnits != "imperial") {
    LOG_WARNF("Invalid locationUnits: %s, resetting to metric", config.locationUnits.c_str());
    config.locationUnits = "metric";
    valid = false;
  }

  return valid;
}

void ConfigManager::resetConfig() {
  LOG_WARN("Resetting configuration to defaults...");
  loadDefaults();
  saveConfig();
}

Config& ConfigManager::getConfig() {
  return config;
}

String ConfigManager::getSchema() {
  return String(FPSTR(SCHEMA_JSON));
}

CRGBPalette16 ConfigManager::getPaletteByIndex(uint8_t index) {
  switch (index) {
    case 0: return RainbowColors_p;
    case 1: return CloudColors_p;
    case 2: return LavaColors_p;
    case 3: return OceanColors_p;
    case 4: return ForestColors_p;
    default: return RainbowColors_p;
  }
}
