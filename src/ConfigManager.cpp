#include "ConfigManager.h"
#include "config.h"
#include "Logger.h"
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
  config.clockColorPaletteIndex = getPaletteIndex(clockColorPalette);
  config.clockColorCharBlend = clockColorCharBlend;
  config.clockColorBlending = (clockColorBlending == LINEARBLEND) ? 1 : 0;
  config.clockSecIndicatorDiff = clockSecIndicatorDiff;
  
  // Open Weather Map
  config.owmApiServer = owmApiServer;
  config.owmApiKey = owmApiKey;
  config.owmLocation = owmLocation;
  config.owmUnits = owmUnits;
  config.owmTempEnabled = owmTempEnabled;
  config.owmTempDisplayTime = owmTempDisplayTime;
  config.owmTempMin = owmTempMin;
  config.owmTempMax = owmTempMax;
  config.owmTempSchedule = owmTempSchedule;
  config.owmUpdateSchedule = owmUpdateSchedule;
  
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
  
  // Open Weather Map
  config.owmApiServer = doc["owmApiServer"] | "api.openweathermap.org";
  config.owmApiKey = doc["owmApiKey"] | "";
  config.owmLocation = doc["owmLocation"] | "";
  config.owmUnits = doc["owmUnits"] | "metric";
  config.owmTempEnabled = doc["owmTempEnabled"] | 1;
  config.owmTempDisplayTime = doc["owmTempDisplayTime"] | 5;
  config.owmTempMin = doc["owmTempMin"] | -40;
  config.owmTempMax = doc["owmTempMax"] | 50;
  config.owmTempSchedule = doc["owmTempSchedule"] | "30 * * * * *";
  config.owmUpdateSchedule = doc["owmUpdateSchedule"] | "0 5 * * * *";
  
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
  
  // Open Weather Map
  doc["owmApiServer"] = config.owmApiServer;
  doc["owmApiKey"] = config.owmApiKey;
  doc["owmLocation"] = config.owmLocation;
  doc["owmUnits"] = config.owmUnits;
  doc["owmTempEnabled"] = config.owmTempEnabled;
  doc["owmTempDisplayTime"] = config.owmTempDisplayTime;
  doc["owmTempMin"] = config.owmTempMin;
  doc["owmTempMax"] = config.owmTempMax;
  doc["owmTempSchedule"] = config.owmTempSchedule;
  doc["owmUpdateSchedule"] = config.owmUpdateSchedule;
  
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
  // Schema embedded in PROGMEM - generated from schema definition
  // This is a minimal schema for now, will be expanded with full UI metadata
  return R"({
    "groups": [
      {
        "id": "wifi",
        "label": "WiFi Settings",
        "fields": [
          {"id": "portalSsid", "type": "text", "label": "Config Portal SSID", "default": "LED-Clock-Config"},
          {"id": "portalPassword", "type": "password", "label": "Config Portal Password", "default": "ledclock"}
        ]
      },
      {
        "id": "clock",
        "label": "Clock Display",
        "fields": [
          {"id": "clockColorMode", "type": "select", "label": "Color Mode", "default": 1, "options": [{"value": 0, "label": "Solid"}, {"value": 1, "label": "Palette"}]},
          {"id": "clockColorSolid", "type": "color", "label": "Solid Color", "default": "#00FF00"},
          {"id": "clockColorPaletteIndex", "type": "select", "label": "Palette", "default": 0, "options": [{"value": 0, "label": "Rainbow"}, {"value": 1, "label": "Cloud"}, {"value": 2, "label": "Lava"}, {"value": 3, "label": "Ocean"}, {"value": 4, "label": "Forest"}]},
          {"id": "clockColorCharBlend", "type": "number", "label": "Char Blend", "default": 5},
          {"id": "clockSecIndicatorDiff", "type": "number", "label": "Second Indicator Dim", "default": 32}
        ]
      },
      {
        "id": "weather",
        "label": "Weather",
        "fields": [
          {"id": "owmApiServer", "type": "text", "label": "API Server", "default": "api.openweathermap.org"},
          {"id": "owmApiKey", "type": "password", "label": "API Key", "default": ""},
          {"id": "owmLocation", "type": "text", "label": "Location", "default": ""},
          {"id": "owmUnits", "type": "select", "label": "Units", "default": "metric", "options": [{"value": "metric", "label": "°C"}, {"value": "imperial", "label": "°F"}]},
          {"id": "owmTempEnabled", "type": "checkbox", "label": "Show Temperature", "default": 1},
          {"id": "owmTempDisplayTime", "type": "number", "label": "Display Seconds", "default": 5}
        ]
      },
      {
        "id": "led",
        "label": "LED Brightness",
        "fields": [
          {"id": "ledBrightness", "type": "number", "label": "Max Brightness", "default": 128},
          {"id": "ledDimEnabled", "type": "checkbox", "label": "Auto Dimming", "default": 1},
          {"id": "ledDimBrightness", "type": "number", "label": "Dimmed Brightness", "default": 64},
          {"id": "ledDimFadeDuration", "type": "number", "label": "Fade Seconds", "default": 30},
          {"id": "ledDimStartTime", "type": "time", "label": "Dim Start", "default": "22:00"},
          {"id": "ledDimEndTime", "type": "time", "label": "Dim End", "default": "06:00"}
        ]
      }
    ]
  })";
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

uint8_t ConfigManager::getPaletteIndex(const CRGBPalette16& palette) {
  // Compare with predefined palettes - this is a simple approach
  // Note: Direct comparison doesn't work well, so we default to Rainbow
  // In practice, the palette index should be stored separately
  return 0;  // Default to Rainbow
}
