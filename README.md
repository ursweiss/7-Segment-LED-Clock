# 7-Segment LED Clock

ESP32-based LED clock with web configuration, weather integration, and auto-dimming.

Arduino code for my beautiful 3D printed LED clock in a retro 7 segment display style.

You can find more details about the project and all downloadable files (STL, 3MF, STEP) [on Printables.com](https://www.printables.com/model/68013-7-segment-led-clock).

## Documentation

- **[API Reference](docs/API.md)** - REST API endpoint documentation
- **[Architecture Overview](docs/ARCHITECTURE.md)** - System components and data flow
- **[Code Review Report](docs/code-review/code-review-report_2025-12-30_1.md)** - Latest comprehensive analysis (Dec 30, 2025)

## Project Structure

```
7-Segment-LED-Clock/
├── .github/
│   └── copilot-instructions.md     # GitHub Copilot development guidelines
├── include/
│   ├── config.h                    # Default configuration values
│   ├── BrightnessControl.h         # Auto-dimming system
│   ├── ConfigManager.h             # Configuration persistence (LittleFS)
│   ├── ConfigStorage.h             # WiFi credentials storage
│   ├── CronHelper.h                # Cron expression parsing
│   ├── LED_Clock.h                 # LED display and character mapping
│   ├── Logger.h                    # Unified logging system
│   ├── schema.h                    # Web UI schema (embedded)
│   ├── version.h                   # Build version tracking
│   ├── Weather.h                   # Open-Meteo API integration
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
- **Open-Meteo Weather Integration**: Fetches temperature every hour via secure HTTPS connection (no API key required)
- **Temperature Display**: Shows temp for 5 seconds at :30 past each minute with color gradient
- **Geolocation Support**: Optional IP-based coordinate detection via ipapi.co for easy setup
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

## Setup Instructions

### 1. Install PlatformIO

- Install [Visual Studio Code](https://code.visualstudio.com/)
- Install [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)

### 2. Initial Configuration (Optional)

The file `include/config.h` contains default settings that are used on first boot. You can optionally edit these values before building, but **all settings can be configured via the web interface** after first boot.

Key defaults in `config.h`:

- WiFi portal password: `"ledclock"`
- LED brightness: `128` (0-255)
- Color mode: `1` (PALETTE)
- Location: Zurich, Switzerland coordinates

**Note:** Once you configure settings via the web interface, they are stored in LittleFS flash memory and persist across reboots. The web interface is the recommended way to configure your clock.

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
1. Display shows "Load" then "ConF"
1. Connect to WiFi network "LED-Clock-Config" (password from config.h)
1. Browser opens to 192.168.4.1 (or open manually)
1. Configure WiFi credentials and timezone
1. Save and restart

### 5. Double Reset to Reconfigure

- Press reset button twice within 10 seconds
- Config portal reopens for reconfiguration

### 6. Web Configuration Access

After WiFi is configured, you can access the web interface for runtime configuration:

1. **Via mDNS** (recommended): http://ledclock.local
1. **Via IP address**: Check your router or serial monitor for the device's IP

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

| Library                 | Version | Purpose                                 |
| ----------------------- | ------- | --------------------------------------- |
| FastLED                 | ^3.6.0  | WS2812 RGB LED control                  |
| ArduinoJson             | ^6.21.4 | JSON parsing for weather API and config |
| TaskScheduler           | ^3.7.0  | Periodic task execution                 |
| ESP_DoubleResetDetector | ^1.3.2  | Double reset detection                  |
| AsyncTCP                | ^1.1.1  | Async TCP for ESP32                     |
| ESPAsyncWebServer       | latest  | Async web server (from GitHub)          |
| ESPAsyncDNSServer       | latest  | Async DNS server for captive portal     |
| ESPAsync_WiFiManager    | ^1.15.1 | WiFi configuration portal               |
| ESP32Time               | ^2.0.6  | RTC (Real-Time Clock) management        |

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

#### Weather Settings

- `locationLatitude`: Latitude in decimal degrees (-90 to 90, e.g., "47.3769")
- `locationLongitude`: Longitude in decimal degrees (-180 to 180, e.g., "8.5417")
- `locationUnits`: "metric" (°C) or "imperial" (°F)
- `weatherTempEnabled`: 1 = show temperature, 0 = disabled
- `weatherTempDisplayTime`: Seconds to display temperature (default: 5)
- `weatherTempMin`: Minimum temperature for color gradient (default: -40)
- `weatherTempMax`: Maximum temperature for color gradient (default: 50)

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
- `weatherTempSchedule`: When to show temp (default: "30 * * * * \*" = :30 past each minute)
- `clockUpdateSchedule`: Clock update rate (default: "\* * * * * \*" = every second)
- `weatherUpdateSchedule`: Weather update rate (default: "0 5 * * * \*" = 5 min past hour)

### Expert Settings

- `FORMAT_FILESYSTEM`: Format LittleFS on boot (default: false)
- `WIFI_RESET_SETTINGS`: Reset all WiFi settings (default: false, debug only)
- `DEBUG`: Uncomment to enable serial debug output
- `_ESPASYNC_WIFIMGR_LOGLEVEL_`: WiFi manager log level (0-4)
- `_ASYNC_HTTP_LOGLEVEL_`: HTTP request log level (0-4)

## Web API Endpoints

The device exposes a RESTful API for configuration and management:

| Endpoint           | Method | Description                                          |
| ------------------ | ------ | ---------------------------------------------------- |
| `/`                | GET    | Web configuration interface (HTML)                   |
| `/api/config`      | GET    | Get current configuration (JSON)                     |
| `/api/config`      | POST   | Update configuration (JSON body)                     |
| `/api/schema`      | GET    | Get configuration schema for web UI                  |
| `/api/version`     | GET    | Get firmware version and device info                 |
| `/api/geolocation` | GET    | Detect approximate coordinates via IP address        |
| `/api/restart`     | POST   | Restart the device                                   |
| `/api/update`      | POST   | Upload firmware for OTA update (multipart/form-data) |

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

## Character Set

The 7-segment display supports:

- **Digits**: 0-9
- **Hex**: A-F
- **Letters**: H, L, N, O, P, R, S, U (upper and lowercase variants)
- **Symbols**: ° (degree), - (minus)
- **Status Words**: "Load", "ConF", "Conn", "Er##" (error codes)

### Error Codes

| Code | Meaning                       | Troubleshooting                               |
| ---- | ----------------------------- | --------------------------------------------- |
| Er01 | WiFi initialization failed    | Check WiFi credentials, try double reset      |
| Er02 | Configuration load/save error | LittleFS filesystem issue, may need format    |
| Er03 | Weather API failure           | Check internet connection, verify coordinates |
| Er04 | Unknown temperature unit      | Check Open-Meteo API response format          |
| Er05 | WiFi disconnected             | Auto-recovery in progress, check router       |

## Task Schedule

| Task               | Interval            | Description                                     |
| ------------------ | ------------------- | ----------------------------------------------- |
| Update Clock       | Every second        | Display current time with blinking colon        |
| Brightness Control | Every second        | Calculate and apply brightness (dimming/fading) |
| Fetch Weather      | Every hour at :05   | Get temperature from Open-Meteo API via HTTPS   |
| Show Temperature   | Every minute at :30 | Display temp for 5 seconds with color gradient  |

## Troubleshooting

### Display shows "Er01"

- WiFi initialization failed
- Check serial monitor for details
- Try double reset to reconfigure WiFi

### Display shows "Er03"

- Weather API request failed
- Check internet connection
- Verify latitude/longitude coordinates are set correctly
- Check serial monitor for detailed error messages
- Ensure ESP32 has sufficient free heap memory for HTTPS/TLS connections

### Display shows "Er04"

- Open-Meteo returned an unexpected temperature unit
- This is rare; check serial monitor for API response details

### Display shows "Er05"

- WiFi connection lost
- Auto-recovery is in progress (attempts reconnection every 5-60 seconds)
- Check your WiFi router is powered on and in range
- Serial monitor shows reconnection attempts and status
- After 30 seconds disconnected, performs full WiFi restart
- If persistent, check for WiFi interference or router issues
- Consider rebooting the ESP32 if recovery doesn't succeed after several minutes

### No temperature display

- Check `weatherTempEnabled = 1` in config.h or web interface
- Verify coordinates are configured (latitude and longitude)
- Use web interface "Detect My Location" button for automatic setup
- Or manually enter coordinates from https://open-meteo.com/en/docs/geocoding-api
- Check serial monitor for API errors (HTTPS connection issues)
- Ensure WiFi is connected
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
- Default values in `config.h` are only used on first boot; web interface settings persist in LittleFS

### Adding Features

1. Keep modular architecture (separate .h/.cpp files)
1. Use TaskScheduler for periodic tasks
1. Add new settings to Config struct in ConfigManager.h
1. Update `config.h` with new default options
1. Update `schema.h` for web UI integration (embedded JSON schema)
1. Update this README with new features
1. Test WiFi resilience and error handling

### Pre-commit Hooks

The project includes pre-commit hooks to ensure code quality:

```bash
# Install pre-commit (once)
pip install pre-commit

# Install hooks for this repository
pre-commit install

# Run manually on all files
pre-commit run --all-files
```

**Automatic checks:**

- Remove trailing whitespace
- Ensure files end with newline
- Consistent line endings (LF)
- Validate JSON/YAML files
- Detect private keys
- Check that `config.h` exists

**Manual checks (run with `pre-commit run --hook <id> --all-files`):**

- `clang-format`: Format C/C++ code (requires clang-format installed)
- `check-flash-usage`: Warn if flash usage exceeds 95%

## License

GNU General Public License v3.0

Copyright (c) 2021 Urs Weiss

## Links

- **Prusa Printers**: https://www.prusaprinters.org/prints/68013-7-segment-led-clock
- **GitHub**: https://github.com/ursweiss/7-Segment-LED-Clock
- **Original Version**: https://github.com/ursweiss/7-Segment-LED-Clock/releases/tag/v1.0.0
- **PlatformIO**: https://platformio.org/
- **FastLED**: https://github.com/FastLED/FastLED
- **Open-Meteo**: https://open-meteo.com/ (free weather API, no key required)
- **ipapi.co**: https://ipapi.co/ (IP-based geolocation service)

## Credits

Original Arduino sketch by Urs Weiss (2021)
Refactored to PlatformIO (2025)

Uses libraries by:

- Khoi Hoang (ESPAsync_WiFiManager, ESP_DoubleResetDetector)
- Daniel Garcia (FastLED)
- Benoit Blanchon (ArduinoJson)
- Anatoli Arkhipenko (TaskScheduler)
- Felix Biego (ESP32Time)
