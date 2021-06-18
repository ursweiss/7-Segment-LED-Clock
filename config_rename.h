/************************/
/**** BASIC SETTINGS ****/
/************************/

//// WiFi
String          portalPassword =          "ledclock";                         // WiFi password needed for config portal

//// Clock
uint8_t         clockColorMode =          1;                                  // 0 => SOLID, 1 => PALETTE, (2 => STATIC_PALETTE - To-Do)
CRGB            clockColorSolid =         CRGB::Green;                        // SOLID mode only: All available predefined colors: https://github.com/FastLED/FastLED/wiki/Pixel-reference#predefined-colors-list
CRGBPalette16   clockColorPalette =       RainbowColors_p;                    // PALETTE mode only: Predefined palette to use (RainbowColors_p, CloudColors_p, LavaColors_p, OceanColors_p, ForestColors_p)
uint8_t         clockColorCharBlend =     5;                                  // PALETTE mode only: Blend single characters by amount n (0-255) | 0 => disabled, >0 amount of change 
TBlendType      clockColorBlending =      LINEARBLEND;                        // PALETTE mode only: options are LINEARBLEND or NOBLEND - linear is 'cleaner'
uint8_t         clockSecIndicatorDiff =   32;                                 // How much to darken down the second indicator when toggling (0-255) | 0 => Disabled

//// Open Weather Map (EXPERIMENTAL)
String          owmApiServer =            "api.openweathermap.org";           // FQDN of API server (normally no need to change it)
String          owmApiKey =               "<your API key>";                   // You OWA API key. Get a free one from https://openweathermap.org/price
String          owmLocation =             "<your city>";                      // The location to get weather data from. API docs: https://openweathermap.org/current#name
String          owmUnits =                "metric";                           // "metric" or "imperial" | If switched to imperial, also chnage the owmTempMin/Max values below in the advanced section (values are limited to two digits, so f.ex. 99°F (~ 37°C) is the maximum possible to show
// OWM Temperature (EXPERIMENTAL)
uint8_t         owmTempEnabled =          0;                                  // Show temperature | 0 => No, 1 => Yes
uint8_t         owmTempDisplayTime =      5;                                  // Show temperature for n seconds

//// FastLED
uint8_t         ledBrightness =           128;                                // Maximum brightness (0-255)


/***************************/
/**** ADVANCED SETTINGS ****/
/***************************/

// Normally there's no need to change these values //

//// WiFi
String          portalSsid =              "LED-Clock-Config";                 // Name of AP when config portal is active
#define         WIFI_SHOW_PW_ON_CONSOLE   false                               // If set to false, it's not hidden everywhere unfortunately

//// Config portal
#define         HTTP_PORT                 80                                  // HTTP port to tuse for config portal
#define         NUM_WIFI_CREDENTIALS      2                                   // How many WiFi credentials should be stored (it connects to the first it finds and tries the next one if it disconnects)
#define         PORTAL_SHOW_PW_ON_CONSOLE false                               // Will show the config portal on console during boot if set to true

//// Open Weather Map (EXPERIMENTAL)
int8_t          owmTempMin =              -40;                                // Min temperature (-99 is min possibl. Value and lower temperature will be shown in blue and fades torwards red if warmer)
int8_t          owmTempMax =              50;                                 // Max temperature (99 is max possible. Value and higher temperature will be shown in red and fades torwards blue if colder)
char            *owmTempSchedule =        "30 * * * * *";                     // When should the temperature be shown in "extended" cron format (see below)

//// FastLED
#define         LED_PIN                   4                                   // LED data pin to use on ESP


/**************************/
/**** EXPERT SETTINGS ****/
/**************************/

// You should only change these values if you know what you're doing //

//// Config portal
#define         DRD_TIMEOUT               10                                  // Number of seconds after reset during which a subseqent reset will be considered a double reset (and enters config portal)
#define         USE_AVAILABLE_PAGES       true                                // Use false if you don't like to display Available Pages in Information Page of Config Portal
#define         USE_ESP_WIFIMANAGER_NTP   true                                // Use false to disable NTP config (you should not disable it, if you want to see the corret time ;-)
#define         USE_CLOUDFLARE_NTP        false                               // Use true to enable CloudFlare NTP service. System can hang if you don't have Internet access while accessing CloudFlare
#define         USING_CORS_FEATURE        false                               // Default false for using only whenever necessary to avoid security issue
#define         USE_CONFIGURABLE_DNS      true                                // Lets you set custom DNS servers
#define         WIFI_RESET_SETTINGS       false                               // Reset all settings (should only be uses for debugging WiFi Manager)
#define         FORMAT_FILESYSTEM         false                               // To format the file system it stores the config on. You only need to format the filesystem once

//// Debugging
//#define         DEBUG                                                       // LED Clock:     Uncomment this line to output debug messages to serial monitor
#define         _ESPASYNC_WIFIMGR_LOGLEVEL_ 1                                 // WiFi Manager:  0 - 4. Higher number, more debugging messages and memory usage
#define         _ASYNC_HTTP_LOGLEVEL_       1                                 // HTTP Reqiest:  0 - 4. Higher number, more debugging messages and memory usage

//// Schedules
// The schedules use an "extended" cron syntax with an addiational seconds field
// <sec> <min> <hour> <day> <month> <day of week>
// Cron docs: https://en.wikipedia.org/wiki/Cron#Overview
char            *clockUpdateSchedule  =   "* * * * * *";                      // When to update clock. Default: Every second (SHOULD NOT be changed)
char            *owmUpdateSchedule    =   "0 5 * * * *";                      // When to update weather data. Default: 5 past every hour (the free OWM only updaes once every full hour)
