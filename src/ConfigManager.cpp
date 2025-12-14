#include "ConfigManager.h"
#include "config.h"
#include "Logger.h"
#include "schema.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

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

  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    LOG_ERROR("Failed to open config file for reading");
    return false;
  }

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

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
  return true;
}

bool ConfigManager::saveConfig() {
  LOG_INFO("Saving configuration to LittleFS...");

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
  LOG_INFO("Configuration saved successfully");
  return true;
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
