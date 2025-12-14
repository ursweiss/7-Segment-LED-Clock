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
