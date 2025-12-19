#ifndef CONFIG_H
#define CONFIG_H
#include <FastLED.h>

/************************/
/**** BASIC SETTINGS ****/
/************************/

// WiFi
inline String           portalPassword =            "ledclock";                         // WiFi password needed for config portal

// Clock
inline uint8_t          clockColorMode =            1;                                  // 0 => SOLID, 1 => PALETTE, (2 => STATIC_PALETTE - To-Do)
inline CRGB             clockColorSolid =           CRGB::Green;                        // SOLID mode only: All available predefined colors: https://github.com/FastLED/FastLED/wiki/Pixel-reference#predefined-colors-list
inline CRGBPalette16    clockColorPalette =         RainbowColors_p;                    // PALETTE mode only: Predefined palette to use (RainbowColors_p, CloudColors_p, LavaColors_p, OceanColors_p, ForestColors_p)
inline uint8_t          clockColorCharBlend =       5;                                  // PALETTE mode only: Blend single characters by amount n (0-255) | 0 => disabled, >0 amount of change
inline TBlendType       clockColorBlending =        LINEARBLEND;                        // PALETTE mode only: options are LINEARBLEND or NOBLEND - linear is 'cleaner'
inline uint8_t          clockSecIndicatorDiff =     32;                                 // How much to darken down the second indicator when toggling (0-255) | 0 => Disabled

// Weather (Open-Meteo API)
inline String           locationLatitude =          "";                                 // Latitude in decimal degrees (-90 to 90). Use "Detect My Location" button in web UI or lookup at https://open-meteo.com/en/docs/geocoding-api
inline String           locationLongitude =         "";                                 // Longitude in decimal degrees (-180 to 180)
inline String           locationUnits =             "metric";                           // "metric" (°C) or "imperial" (°F) | If switched to imperial, also change weatherTempMin/Max values below (limited to two digits, max 99°F ~ 37°C)

//// Weather Temperature Display
inline uint8_t          weatherTempEnabled =        1;                                  // Show temperature | 0 => No, 1 => Yes
inline uint8_t          weatherTempDisplayTime =    5;                                  // Show temperature for n seconds

// FastLED
inline uint8_t          ledBrightness =             128;                                // Maximum brightness (0-255)
inline uint8_t          ledDimEnabled =             1;                                  // Enable automatic dimming | 0 => No, 1 => Yes
inline uint8_t          ledDimBrightness =          64;                                 // Dimmed brightness level (0-255)
inline uint8_t          ledDimFadeDuration =        30;                                 // How long (in seconds) the fading between states (normal <-> dimmed) should take
inline const char*      ledDimStartTime =           "22:00";                            // Time to start dimming (HH:MM, 24-hour format)
inline const char*      ledDimEndTime =             "06:00";                            // Time to end dimming (HH:MM, 24-hour format)

/***************************/
/**** ADVANCED SETTINGS ****/
/***************************/

// Normally there's no need to change these values //

// WiFi
inline String           portalSsid =                "LED-Clock-Config";                 // Name of AP when config portal is active
#define                 WIFI_SHOW_PW_ON_CONSOLE     false                               // If set to false, it's not hidden everywhere unfortunately

// Config portal
#define                 HTTP_PORT                   80                                  // HTTP port to use for config portal
#define                 NUM_WIFI_CREDENTIALS        1                                   // How many WiFi credentials should be stored (if >1 it connects to the first it finds and tries the next one if it disconnects)
#define                 PORTAL_SHOW_PW_ON_CONSOLE   false                               // Will show the config portal on console during boot if set to true

// Weather (Open-Meteo API)
inline int8_t           weatherTempMin =            -40;                                // Min temperature (-99 is min possible. Value and lower temperature will be shown in blue and fades towards red if warmer)
inline int8_t           weatherTempMax =            50;                                 // Max temperature (99 is max possible. Value and higher temperature will be shown in red and fades towards blue if colder)
inline const char*      weatherTempSchedule =       "30 * * * * *";                     // When should the temperature be shown in "extended" cron format (at 30 seconds every minute - see below)

// FastLED
#define                 LED_PIN                     4                                   // LED data pin to use on ESP


/**************************/
/**** EXPERT SETTINGS ****/
/**************************/

// You should only change these values if you know what you're doing //

// Config portal
#define                 DRD_TIMEOUT                 10                                  // Number of seconds after reset during which a subsequent reset will be considered a double reset (and enters config portal)
#define                 DRD_ADDRESS                 0

// WiFi reconnection settings
#define                 WIFI_RECONNECT_INTERVAL_MS  5000                                // Initial retry interval (5 seconds)
#define                 WIFI_MAX_RECONNECT_ATTEMPTS 10                                  // Max attempts before full WiFi restart
#define                 WIFI_MAX_BACKOFF_MS         60000                               // Maximum backoff interval (1 minute)
#define                 WIFI_FULL_RESTART_THRESHOLD 30000                               // Trigger full restart after 30 seconds disconnected
#define                 ESP_DRD_USE_LITTLEFS        true
#define                 ESP_DRD_USE_SPIFFS          false
#define                 ESP_DRD_USE_EEPROM          false
#define                 USE_AVAILABLE_PAGES         false                               // Use false if you don't like to display Available Pages in Information Page of Config Portal
#define                 USE_ESP_WIFIMANAGER_NTP     true                                // Use false to disable NTP config (you should not disable it, if you want to see the correct time ;-)
#define                 USE_CLOUDFLARE_NTP          false                               // Use true to enable CloudFlare NTP service. System can hang if you don't have Internet access while accessing CloudFlare
#define                 USING_CORS_FEATURE          false                               // Default false for using only whenever necessary to avoid security issue
#define                 USE_CONFIGURABLE_DNS        true                                // Lets you set custom DNS servers
#define                 WIFI_RESET_SETTINGS         false                               // Reset all settings (should only be used for debugging WiFi Manager)
#define                 FORMAT_FILESYSTEM           false                               // To format the file system it stores the config on. You only need to format the filesystem once

// Debugging
//#define               DEBUG                                                           // LED Clock:     Uncomment this line to output debug messages to serial monitor
#define                 _ESPASYNC_WIFIMGR_LOGLEVEL_ 1                                   // WiFi Manager:  0 - 4. Higher number, more debugging messages and memory usage
#define                 _ASYNC_HTTP_LOGLEVEL_       1                                   // HTTP Request:  0 - 4. Higher number, more debugging messages and memory usage

// Schedules

// The schedules use an "extended" cron syntax with an additional seconds field
// <sec> <min> <hour> <day> <month> <day of week>
// Cron docs: https://en.wikipedia.org/wiki/Cron#Overview

// Supported syntax:
//   - Wildcard: * (matches any value)
//   - Single value: 5 (matches only that value)
//   - Step values: */15 (every 15, starting from 0) or 5/15 (every 15, starting from 5)
// Examples:
//   "0 */15 * * * *"    = Every 15 minutes at 0 seconds (00:00, 00:15, 00:30, 00:45)
//   "0 5/15 * * * *"    = Every 15 minutes starting from 05 (00:05, 00:20, 00:35, 00:50)
//   "0 0 8/2 * * *"     = Every 2 hours starting from 8:00 (08:00, 10:00, 12:00, 14:00, etc.)
// NOTE: Ranges (e.g. 1-5) and lists (e.g. 1,2,3,4,5) are NOT supported
inline const char*      clockUpdateSchedule  =      "* * * * * *";                      // When to update clock. Default: Every second (SHOULD NOT be changed)
inline const char*      weatherUpdateSchedule =     "0 5/15 * * * *";                   // When to update weather data. Default: Every 15 minutes starting from 5 past every hour

#endif // CONFIG_H
