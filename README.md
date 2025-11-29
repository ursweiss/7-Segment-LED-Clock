# 7-Segment LED Clock - PlatformIO Migration

This project has been successfully refactored from Arduino IDE to PlatformIO with modern libraries and clean modular architecture.

## Project Structure

```
7-Segment-LED-Clock/
├── .github/
│   └── copilot-instructions.md    # GitHub Copilot development guidelines
├── _legacy/                        # Original Arduino sketch (DO NOT MODIFY)
│   └── 7-Segment-LED-Clock/
├── include/
│   ├── config.h                    # Your configuration (gitignored)
│   ├── config_rename.h             # Configuration template
│   ├── BrightnessControl.h         # Auto-dimming system
│   ├── ConfigManager.h             # Configuration persistence (LittleFS)
│   ├── ConfigStorage.h             # WiFi credentials storage
│   ├── CronHelper.h                # Cron expression parsing
│   ├── LED_Clock.h                 # LED display and character mapping
│   ├── Logger.h                    # Unified logging system
│   ├── schema.h                    # Web UI schema (embedded)
│   ├── version.h                   # Build version tracking
│   ├── Weather.h                   # OpenWeatherMap API integration
│   ├── WebConfig.h                 # Web configuration server
│   └── WiFi_Manager.h              # WiFi and NTP configuration
├── src/
│   ├── main.cpp                    # Main program with TaskScheduler
│   ├── BrightnessControl.cpp       # Auto-dimming implementation
│   ├── ConfigManager.cpp           # Configuration management
│   ├── ConfigStorage.cpp           # WiFi credentials handling
│   ├── CronHelper.cpp              # Cron utilities
│   ├── LED_Clock.cpp               # LED display implementation
│   ├── Logger.cpp                  # Logging implementation
│   ├── Weather.cpp                 # Weather API implementation (HTTPS)
│   ├── WebConfig.cpp               # Web server and API endpoints
│   ├── web_html.h                  # Web UI HTML/CSS/JS (embedded)
│   └── WiFi_Manager.cpp            # WiFi/NTP implementation
├── .gitignore
└── platformio.ini                  # PlatformIO configuration
```

## Hardware

- **Board**: ESP32-WROOM-32 (configured as `esp32dev`)
- **LEDs**: 58 WS2812 RGB LEDs
  - 4 characters × 7 segments × 2 LEDs = 56 LEDs
  - 2 LEDs for colon/second indicator
- **Data Pin**: GPIO 4 (configurable in `config.h`)

## Features

### ✅ Implemented
- **WiFi Manager**: Auto-configuration portal with ESPAsync_WiFiManager
- **Double Reset Detection**: Press reset twice within 10 seconds to enter config portal
- **NTP Time Sync**: Automatic timezone-aware time synchronization
- **7-Segment Display**: Custom character mapping for digits, letters, and symbols
- **Color Modes**:
  - SOLID: Single color (e.g., Green, Blue, Red)
  - PALETTE: Rainbow/animated color palettes with per-character blending
- **Second Indicator**: Blinking colon LEDs with configurable dimming
- **OpenWeatherMap Integration**: Fetches temperature every hour via secure HTTPS connection
- **Temperature Display**: Shows temp for 5 seconds at :30 past each minute
- **Task Scheduling**: Replaced CronAlarms with TaskScheduler for reliability
- **Web Configuration Interface**: Runtime configuration via web browser at http://ledclock.local
  - Live configuration editing with real-time validation
  - API key masking for security
  - OTA firmware updates via web interface
  - Persistent storage with LittleFS
  - RESTful API for programmatic control
- **Auto-Brightness Dimming**: Automatic brightness reduction during night hours with smooth fade transitions
  - Configurable dim period (start/end times)
  - Smooth brightness fading with adjustable duration
  - Independent colon brightness control

### 🔮 Future Enhancements
- Additional display modes and animations
- Weather forecast display
- Multi-timezone support

## Setup Instructions

### 1. Install PlatformIO
- Install [Visual Studio Code](https://code.visualstudio.com/)
- Install [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)

### 2. Configure Your Clock
1. Copy `include/config_rename.h` to `include/config.h`
2. Edit `include/config.h`:
   ```cpp
   // WiFi portal password
   String portalPassword = "your_password";
   
   // OpenWeatherMap settings
   String owmApiKey = "your_api_key";      // Get free key from openweathermap.org
   String owmLocation = "YourCity,CC";     // e.g., "Zurich,CH"
   
   // LED brightness (0-255)
   uint8_t ledBrightness = 128;
   
   // Color mode: 0 = SOLID, 1 = PALETTE
   uint8_t clockColorMode = 1;
   ```

**Note:** Settings in `config.h` are initial defaults. Once configured via the web interface, settings are stored in LittleFS and persist across reboots. Web interface settings take precedence over `config.h`.

### 3. Build and Upload
```bash
# Build the project
platformio run

# Upload to ESP32
platformio run --target upload

# Monitor serial output
platformio device monitor
```

### 4. First-Time WiFi Setup
1. Power on the ESP32
2. Display shows "Load" then "ConF"
3. Connect to WiFi network "LED-Clock-Config" (password from config.h)
4. Browser opens to 192.168.4.1 (or open manually)
5. Configure WiFi credentials and timezone
6. Save and restart

### 5. Double Reset to Reconfigure
- Press reset button twice within 10 seconds
- Config portal reopens for reconfiguration

### 6. Web Configuration Access

After WiFi is configured, you can access the web interface for runtime configuration:

1. **Via mDNS** (recommended): http://ledclock.local
2. **Via IP address**: Check your router or serial monitor for the device's IP

The web interface provides:
- Real-time configuration editing with validation
- All settings from `config.h` can be modified
- Firmware updates (OTA) without USB connection
- Device restart capability
- Configuration changes are saved to flash memory
- API key values are masked for security

**Note:** Some settings require a device restart to take effect (indicated in the web UI).

## Libraries Used

All libraries are automatically managed by PlatformIO:

| Library | Version | Purpose |
|---------|---------|------|
| FastLED | ^3.6.0 | WS2812 RGB LED control |
| ArduinoJson | ^6.21.4 | JSON parsing for weather API and config |
| TaskScheduler | ^3.7.0 | Periodic task execution |
| ESP_DoubleResetDetector | ^1.3.2 | Double reset detection |
| AsyncTCP | ^1.1.1 | Async TCP for ESP32 |
| ESPAsyncWebServer | latest | Async web server (from GitHub) |
| ESPAsyncDNSServer | latest | Async DNS server for captive portal |
| ESPAsync_WiFiManager | ^1.15.1 | WiFi configuration portal |
| ESP32Time | ^2.0.6 | RTC (Real-Time Clock) management |

## Configuration Reference

### Basic Settings

#### WiFi
- `portalPassword`: Password for config portal (default: "ledclock")
- `portalSsid`: Config portal SSID (default: "LED-Clock-Config")

#### Clock Display
- `clockColorMode`: 0 = SOLID, 1 = PALETTE
- `clockColorSolid`: Color for SOLID mode (e.g., `CRGB::Green`)
- `clockColorPaletteIndex`: Palette for PALETTE mode (0=Rainbow, 1=Cloud, 2=Lava, 3=Ocean, 4=Forest)
- `clockColorCharBlend`: Per-character color offset (0-255)
- `clockColorBlending`: LINEARBLEND or NOBLEND
- `clockSecIndicatorDiff`: Second indicator dimming (0-255, 0=disabled)

#### OpenWeatherMap
- `owmApiServer`: API server (default: "api.openweathermap.org")
- `owmApiKey`: Your API key from openweathermap.org
- `owmLocation`: City name (e.g., "London,UK")
- `owmUnits`: "metric" or "imperial"
- `owmTempEnabled`: 1 = show temperature, 0 = disabled
- `owmTempDisplayTime`: Seconds to display temperature (default: 5)

#### LED Hardware
- `LED_PIN`: GPIO pin for LED data (default: 4)
- `ledBrightness`: Overall brightness (0-255, default: 128)

#### Brightness Control
- `ledDimEnabled`: Enable automatic dimming (0=disabled, 1=enabled, default: 1)
- `ledDimBrightness`: Brightness level during dim period (0-255, default: 64)
- `ledDimStartTime`: When to start dimming (format: "HH:MM", default: "22:00")
- `ledDimEndTime`: When to end dimming (format: "HH:MM", default: "06:00")
- `ledDimFadeDuration`: Fade transition time in seconds (default: 30)

### Advanced Settings
- `HTTP_PORT`: Config portal port (default: 80)
- `NUM_WIFI_CREDENTIALS`: Number of WiFi networks to store (default: 2)
- `DRD_TIMEOUT`: Double reset timeout in seconds (default: 10)
- `owmTempMin/Max`: Temperature range for color gradient (-40 to 50°C)
- `owmTempSchedule`: When to show temp (default: "30 * * * * *" = :30 past each minute)
- `clockUpdateSchedule`: Clock update rate (default: "* * * * * *" = every second)
- `owmUpdateSchedule`: Weather update rate (default: "0 5 * * * *" = 5 min past hour)

### Expert Settings
- `FORMAT_FILESYSTEM`: Format LittleFS on boot (default: false)
- `WIFI_RESET_SETTINGS`: Reset all WiFi settings (default: false, debug only)
- `DEBUG`: Uncomment to enable serial debug output
- `_ESPASYNC_WIFIMGR_LOGLEVEL_`: WiFi manager log level (0-4)
- `_ASYNC_HTTP_LOGLEVEL_`: HTTP request log level (0-4)

## Web API Endpoints

The device exposes a RESTful API for configuration and management:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Web configuration interface (HTML) |
| `/api/config` | GET | Get current configuration (JSON) |
| `/api/config` | POST | Update configuration (JSON body) |
| `/api/schema` | GET | Get configuration schema for web UI |
| `/api/version` | GET | Get firmware version and device info |
| `/api/restart` | POST | Restart the device |
| `/api/update` | POST | Upload firmware for OTA update (multipart/form-data) |

### API Examples

**Get Current Configuration:**
```bash
curl http://ledclock.local/api/config
```

**Update Configuration:**
```bash
curl -X POST http://ledclock.local/api/config \
  -H "Content-Type: application/json" \
  -d '{"ledBrightness":200,"clockColorMode":1}'
```

**Get Device Information:**
```bash
curl http://ledclock.local/api/version
```

**Restart Device:**
```bash
curl -X POST http://ledclock.local/api/restart
```

**OTA Firmware Update:**
```bash
curl -X POST http://ledclock.local/api/update \
  -F "firmware=@.pio/build/esp32dev/firmware.bin"
```

**Note:** API key values are masked in GET responses for security (e.g., `owmApiKey` shows as `***...***`).

## Character Set

The 7-segment display supports:
- **Digits**: 0-9
- **Hex**: A-F
- **Letters**: H, L, N, O, P, R, S, U (upper and lowercase variants)
- **Symbols**: ° (degree), - (minus)
- **Status Words**: "Load", "ConF", "Conn", "Er##" (error codes)

## Task Schedule

| Task | Interval | Description |
|------|----------|-------------|
| Update Clock | Every second | Display current time with blinking colon |
| Brightness Control | Every second | Calculate and apply brightness (dimming/fading) |
| Fetch Weather | Every hour at :05 | Get temperature from OpenWeatherMap via HTTPS |
| Show Temperature | Every minute at :30 | Display temp for 5 seconds with color gradient |

## Troubleshooting

### Display shows "Err01"
- WiFi initialization failed
- Check serial monitor for details
- Try double reset to reconfigure WiFi

### No temperature display
- Check `owmTempEnabled = 1` in config.h or web interface
- Verify API key is valid at openweathermap.org
- Check serial monitor for API errors (HTTPS connection issues)
- Ensure WiFi is connected
- Verify ESP32 has sufficient free heap memory for HTTPS/TLS connections
- Weather updates occur at :05 past each hour (check timing)

### Web interface not accessible
- Verify device is connected to WiFi (check serial monitor)
- Try both http://ledclock.local and the IP address
- Check if mDNS is supported on your network (some corporate networks block it)
- Ensure device and your computer are on the same network
- Check that port 80 is not blocked by firewall

### Config portal not appearing
- Double reset detection: Press reset twice within 10 seconds
- First boot automatically enters config portal
- Check LED display shows "ConF"

### LEDs not lighting up
- Verify `LED_PIN` matches your wiring (default: GPIO 4)
- Check LED power supply (5V, sufficient current for 58 LEDs)
- Increase brightness: `ledBrightness = 255`

### Time not updating
- NTP sync requires WiFi connection
- Check timezone configuration in config portal
- Serial monitor shows NTP sync status
- First sync may take 30-60 seconds

## Development

### Code Style
- Single blank line between functions/blocks
- No trailing whitespace
- Use `const char*` over Arduino `String` where possible
- Use ConfigManager for all runtime settings (avoid global variables)
- Follow logging conventions with Logger macros (LOG_INFO, LOG_ERROR, LOG_DEBUG)
- All config options must be in both `config.h` and `config_rename.h`
- **Never modify `_legacy/` folder**

### Adding Features
1. Keep modular architecture (separate .h/.cpp files)
2. Use TaskScheduler for periodic tasks
3. Add new settings to Config struct in ConfigManager.h
4. Update both `config.h` and `config_rename.h` with new options
5. Update `schema.h` for web UI integration (embedded JSON schema)
6. Update this README with new features
7. Test WiFi resilience and error handling

## License

GNU General Public License v3.0

Copyright (c) 2021 Urs Weiss

## Links

- **Prusa Printers**: https://www.prusaprinters.org/prints/68013-7-segment-led-clock
- **GitHub**: https://github.com/ursweiss/7-Segment-LED-Clock
- **PlatformIO**: https://platformio.org/
- **FastLED**: https://github.com/FastLED/FastLED
- **OpenWeatherMap**: https://openweathermap.org/

## Credits

Original Arduino sketch by Urs Weiss (2021)  
Refactored to PlatformIO (2025)

Uses libraries by:
- Khoi Hoang (ESPAsync_WiFiManager, ESP_DoubleResetDetector)
- Daniel Garcia (FastLED)
- Benoit Blanchon (ArduinoJson)
- Anatoli Arkhipenko (TaskScheduler)
- Felix Biego (ESP32Time)
