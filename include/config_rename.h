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

// Open Weather Map
inline String           owmApiServer =              "api.openweathermap.org";           // FQDN of API server (normally no need to change it)
inline String           owmApiKey =                 "<your API key>";                   // Your OWM API key. Get a free one from https://openweathermap.org/price
inline String           owmLocation =               "<your city>";                      // The location to get weather data from. API docs: https://openweathermap.org/current#name
inline String           owmUnits =                  "metric";                           // "metric" or "imperial" | If switched to imperial, also change the owmTempMin/Max values below in the advanced section (values are limited to two digits, so f.ex. 99°F (~ 37°C) is the maximum possible to show

//// OWM Temperature
inline uint8_t          owmTempEnabled =            1;                                  // Show temperature | 0 => No, 1 => Yes
inline uint8_t          owmTempDisplayTime =        5;                                  // Show temperature for n seconds

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

// Open Weather Map
inline int8_t           owmTempMin =                -40;                                // Min temperature (-99 is min possible. Value and lower temperature will be shown in blue and fades towards red if warmer)
inline int8_t           owmTempMax =                50;                                 // Max temperature (99 is max possible. Value and higher temperature will be shown in red and fades towards blue if colder)
inline const char*      owmTempSchedule =           "30 * * * * *";                     // When should the temperature be shown in "extended" cron format (see below)

// FastLED
#define                 LED_PIN                     4                                   // LED data pin to use on ESP


/**************************/
/**** EXPERT SETTINGS ****/
/**************************/

// You should only change these values if you know what you're doing //

// Config portal
#define                 DRD_TIMEOUT                 10                                  // Number of seconds after reset during which a subsequent reset will be considered a double reset (and enters config portal)
#define                 DRD_ADDRESS                 0
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

// NOTE: Using ranges (e.g. 1-5) lists (e.g. 1,2,3,4,5) and step values (e.g. */5) are NOT supported
inline const char*      clockUpdateSchedule  =      "* * * * * *";                      // When to update clock. Default: Every second (SHOULD NOT be changed)
inline const char*      owmUpdateSchedule =         "0 5 * * * *";                      // When to update weather data. Default: 5 past every hour (the free OWM only updates once every full hour)

#endif // CONFIG_H