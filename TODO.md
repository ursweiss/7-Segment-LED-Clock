# TODO List

Future improvements prioritized by impact and effort. This list excludes work already completed during code review cleanup phases (error handling, performance optimizations, refactoring, copyright headers).

______________________________________________________________________

## 🔴 Critical Priority

### Implement Unit Testing Framework

**Category:** Testing
**Effort:** Large
**Description:** Create comprehensive test suite for embedded code. Currently there are NO tests in the `test/` directory.

**Implementation:**

- Set up PlatformIO native testing or hardware-in-the-loop tests
- Add tests for: ConfigValidator, CronHelper, LED character mapping, color calculations
- Test error handling paths
- Mock ESP32 dependencies for unit testing
- Add CI/CD integration (GitHub Actions)
- Aim for >70% code coverage on testable components

**Reference:** Section 8 of code review report (Critical gap)

______________________________________________________________________

### Add Proper TLS Certificate Validation

**Category:** Security
**Effort:** Medium
**Description:** Currently using `client.setInsecure()` for HTTPS connections to ipapi.co geolocation service and open-meteo.com weather service. This bypasses certificate validation and is vulnerable to MITM attacks.

**Implementation:**

- Embed GTS Root R4 certificate (Google Trust Services) to WebConfig.cpp
- Use `client.setCACert(rootCA)` instead of `client.setInsecure()`
- Certificate valid until: January 28, 2028
- Increases flash usage by ~1-2KB

**Files to modify:**

- `src/WebConfig.cpp` - Add certificate for geolocation endpoint
- `src/Weather.cpp` - Add certificate for weather endpoint (now uses SecureHTTPClient)

**Reference:** Section 2.1 of code review report (Critical security issue)

______________________________________________________________________

## 🟡 High Priority

### Implement API Authentication

**Category:** Security
**Effort:** Medium
**Description:** All REST API endpoints are currently unauthenticated. Anyone on the same network can reconfigure the device or upload malicious firmware.

**Implementation:**

- Add session-based authentication or API key
- Use same password as WiFi config portal
- Add authentication headers to all /api/\* endpoints
- Protect OTA update endpoint
- Add CORS protection
- Implement rate limiting on sensitive endpoints

**Files to modify:**

- `src/WebConfig.cpp` - Add auth middleware
- `src/web_html.h` - Update web UI to handle authentication
- `include/schema.h` - Add auth settings to schema

**Reference:** Section 2.4 of code review report

______________________________________________________________________

### Improve Error Handling with Retry Logic

**Category:** Robustness
**Effort:** Medium
**Description:** JSON parsing failures, weather API failures, and geolocation failures are currently not retried. Add exponential backoff retry logic.

**Implementation:**

- Add retry mechanism to `Weather::fetchWeather()` (3 attempts with backoff)
- Add retry to geolocation API in `WebConfig.cpp`
- Add JSON schema validation before parsing
- Improve error messages in web UI
- Log full payloads on parse errors (DEBUG mode only)

**Files to modify:**

- `src/Weather.cpp` - Add retry logic
- `src/WebConfig.cpp` - Add retry for geolocation
- `src/ConfigManager.cpp` - Add JSON schema validation

**Reference:** Section 3.1 of code review report

______________________________________________________________________

### Standardize Error Handling

**Category:** Code Quality
**Effort:** Medium
**Description:** Currently inconsistent error handling patterns (booleans, error codes in temperature variable, displayError codes, ESP.restart on critical). Standardize approach.

**Implementation:**

- Create error enum class (Success, WiFiInitFailed, ConfigLoadFailed, etc.)
- Create Result<T> struct for functions that can fail
- Update all error-prone functions to return Result
- Update LED display error handling to use enum
- Document all error codes in README

**Files to modify:**

- Create `include/ErrorCodes.h`
- Refactor: `ConfigManager`, `Weather`, `WiFi_Manager`, `LED_Clock`

**Reference:** Section 5.4 of code review report

______________________________________________________________________

### Add Factory Reset Endpoint

**Category:** Usability
**Effort:** Small
**Description:** Add /api/reset endpoint to web UI for easy factory reset without needing physical access to device.

**Implementation:**

- Add POST /api/reset endpoint
- Erase LittleFS config file
- Reset to defaults from `config.h`
- Add factory reset button to web UI
- Add long button press for physical factory reset

**Files to modify:**

- `src/WebConfig.cpp` - Add endpoint
- `src/web_html.h` - Add UI button
- `src/ConfigManager.cpp` - Add reset method

**Reference:** Section 3.3 of code review report

______________________________________________________________________

### Add Debug Output Toggle to Web UI

**Category:** Debugging & User Support
**Effort:** Medium
**Description:** Currently, debug output is controlled by `#define DEBUG` in `include/config.h`, requiring recompilation. Add runtime toggle so users can enable debug output without recompiling.

**Implementation:**

- Add `debugEnabled` boolean field to Config struct in `include/ConfigManager.h`
- Update `include/schema.h` to add debug toggle to "Advanced" settings group
- Modify `Logger.h` to check runtime config instead of just compile-time `#define DEBUG`
- Add logic to persist debug setting to LittleFS
- Update README.md to document the debug feature

**Benefits:**

- Users can enable debug output for troubleshooting
- Debug logs can be captured via serial monitor
- Useful for reporting bugs with detailed information

**Files to modify:**

- `include/ConfigManager.h` - Add debugEnabled field
- `include/schema.h` - Add UI field
- `include/Logger.h` - Update macros for runtime check
- `src/ConfigManager.cpp` - Handle new field

**Reference:** Existing TODO item (Medium priority)

______________________________________________________________________

## 🟢 Medium Priority

### Add Input Validation to API Endpoints

**Category:** Security
**Effort:** Medium
**Description:** POST /api/config endpoint accepts JSON without thorough validation. Add comprehensive validation.

**Implementation:**

- Validate all string lengths against defined limits
- Validate cron expression syntax before accepting (use CronHelper)
- Validate time format (HH:MM) before saving
- Add regex validation for hex colors
- Validate numeric ranges
- Return specific error messages for validation failures

**Files to modify:**

- `src/WebConfig.cpp` - Add validation before saving config
- Use existing `ConfigValidator` class more comprehensively

**Reference:** Section 2.3 of code review report

______________________________________________________________________

### Optimize String Usage for Memory

**Category:** Performance
**Effort:** Large
**Description:** Heavy use of Arduino String class risks heap fragmentation. Refactor Config struct to use fixed-size buffers where appropriate.

**Implementation:**

- Replace String with char arrays for: `ledDimStartTime`, `ledDimEndTime`, `locationUnits`
- Use const char\* for read-only strings
- Profile heap fragmentation after days of runtime
- Keep String for variable-length fields (schedules, coordinates)

**Example:**

```cpp
// Instead of:
String ledDimStartTime;  // 32 bytes + allocation

// Use:
char ledDimStartTime[6];  // "HH:MM\0" - 6 bytes fixed
```

**Files to modify:**

- `include/ConfigManager.h` - Refactor Config struct
- `src/ConfigManager.cpp` - Update serialization/deserialization
- All files using Config fields

**Reference:** Section 4.1 of code review report

______________________________________________________________________

### Cache Palette Updates

**Category:** Performance
**Effort:** Small
**Description:** Palette updates are called redundantly even when palette hasn't changed. Cache palette index and only update on change.

**Implementation:**

- Cache current palette index
- Set dirty flag when config changes via API
- Only call `updatePaletteFromConfig()` when index changes
- Avoid redundant `getPaletteByIndex()` calls

**Files to modify:**

- `src/LED_Clock.cpp` - Add caching logic

**Reference:** Section 4.3 of code review report

______________________________________________________________________

### Implement HTTP Client Connection Reuse

**Category:** Performance
**Effort:** Medium
**Description:** Currently creates new WiFiClientSecure and HTTPClient on every weather fetch, causing unnecessary TLS handshakes.

**Implementation:**

- Create persistent weather client at module scope
- Reuse connection if recent (\< 5 min)
- Only teardown on timeout or error
- Reduces weather fetch time by 2-3 seconds

**Files to modify:**

- `src/Weather.cpp` - Refactor to use persistent client
- Update `SecureHTTPClient` to support connection reuse

**Reference:** Section 4.4 of code review report

______________________________________________________________________

### Reduce Task Scheduler Frequency

**Category:** Performance
**Effort:** Small
**Description:** Clock updates every 100ms but only changes every second. Optimize update frequency.

**Implementation:**

- Reduce task interval to 500ms or 1000ms
- Trigger immediate update on second change
- Consider separate tasks for brightness (100ms) and display (1000ms)

**Files to modify:**

- `src/main.cpp` - Adjust task intervals

**Reference:** Section 4.6 of code review report

______________________________________________________________________

### Add Watchdog Feeding During WiFi Reconnection

**Category:** Robustness
**Effort:** Small
**Description:** WiFi reconnection with exponential backoff could take ~10 minutes and trigger hardware watchdog reset.

**Implementation:**

- Add explicit watchdog feeding in reconnection loop
- Reduce max reconnection time or attempts
- Display reconnection status on LED clock

**Files to modify:**

- `src/WiFi_Manager.cpp` - Add watchdog feeding

**Reference:** Section 3.4 of code review report

______________________________________________________________________

### Add NULL Pointer Checks in Logger

**Category:** Robustness
**Effort:** Small
**Description:** `logMessage()` calls `loggerRTC->getYear()` without NULL check when `timestampAvailable` is true. Add safety checks.

**Implementation:**

- Add NULL checks before all `loggerRTC` dereferences
- Handle case where RTC becomes invalid after initial check

**Files to modify:**

- `src/Logger.cpp` - Add NULL safety

**Reference:** Section 3.2 of code review report

______________________________________________________________________

### Add Task Scheduler Health Monitoring

**Category:** Robustness
**Effort:** Small
**Description:** Task scheduler execution failures are silent. Add health checking and recovery.

**Implementation:**

- Check task execution status
- Log task failures
- Implement recovery logic (task restart or device restart)
- Display task failure on LED clock

**Files to modify:**

- `src/main.cpp` - Add task monitoring

**Reference:** Section 3.6 of code review report

______________________________________________________________________

### Refactor Large Functions

**Category:** Code Quality
**Effort:** Medium
**Description:** Some functions remain complex and should be split further for testability.

**Candidates:**

- `displayTemperature()` in LED_Clock.cpp (76 lines, complexity 12) - **Note:** Partially addressed in Phase 5, may need further splitting
- Color calculation duplication in indicator functions - **Note:** Addressed in Phase 5 with ColorCalculator

**Implementation:**

- Extract helper functions for formatting, color calculation, display logic
- Reduce cyclomatic complexity to \< 10
- Make functions unit-testable

**Files to review:**

- `src/LED_Clock.cpp`
- `src/ConfigManager.cpp`

**Reference:** Section 5.1 of code review report

______________________________________________________________________

### Replace Magic Numbers with Named Constants

**Category:** Code Quality
**Effort:** Small
**Description:** Replace hardcoded numbers throughout codebase with named constants.

**Examples:**

- Temperature color indices (160, 0)
- RTC sync interval (3600000)
- HTTP timeout (5000)
- Epoch threshold (1000000000)

**Implementation:**

- Define constexpr constants at file or class scope
- Use descriptive names
- Add comments explaining values

**Files to modify:**

- `src/LED_Clock.cpp`, `src/main.cpp`, `src/Weather.cpp`, `src/BrightnessControl.cpp`

**Reference:** Section 5.2 of code review report

______________________________________________________________________

### Improve Password Security

**Category:** Security
**Effort:** Medium
**Description:** Default portal password "ledclock" is hardcoded and visible in source. Improve security.

**Implementation:**

- Generate random password on first boot
- Display password on LED clock during setup
- Force password change on first web UI access
- Add password strength meter in web UI
- Store hashed passwords instead of plaintext (if feasible on ESP32)

**Files to modify:**

- `include/config.h` - Remove hardcoded default
- `src/WiFi_Manager.cpp` - Generate random password
- `src/LED_Clock.cpp` - Display password during setup
- `src/web_html.h` - Add password change UI

**Reference:** Section 2.2 of code review report

______________________________________________________________________

## 🟢 Low Priority

### Add STATIC_PALETTE Color Mode

**Category:** Display Features
**Effort:** Small
**Description:** Implement static palette mode (mode 2) for clock color display.

**Current Modes:**

- Mode 0: SOLID - Single solid color
- Mode 1: PALETTE - Animated rainbow/palette effects with per-character blending
- Mode 2: STATIC_PALETTE - *Not yet implemented*

**Implementation:**

- Add new case to color mode switch in `LED_Clock.cpp`
- Define static palette color progression
- Update web UI schema in `include/schema.h`
- Update README.md with new mode documentation

**Files to modify:**

- `src/LED_Clock.cpp` - Add mode 2 implementation
- `include/schema.h` - Update UI
- README.md - Document feature

**Reference:** Existing TODO item (Low priority)

______________________________________________________________________

### Extract Common Indicator Color Calculation

**Category:** Code Quality
**Effort:** Small
**Description:** `toggleSecondIndicator()`, `secondIndicatorOn()`, `secondIndicatorDim()` have similar color calculation logic. **Note:** May be partially addressed by ColorCalculator in Phase 5.

**Implementation:**

- Review if ColorCalculator already covers this
- Extract remaining duplicated logic if needed

**Files to modify:**

- `src/LED_Clock.cpp` - Check current state and refactor if needed

**Reference:** Section 5.5 of code review report

______________________________________________________________________

### Improve Function Parameter Design

**Category:** Code Quality
**Effort:** Small
**Description:** `displayCharacter()` has long parameter list with multiple optional parameters. Use options struct.

**Implementation:**

```cpp
struct DisplayOptions {
  bool customize = false;
  CRGBPalette16 palette = RainbowColors_p;
  uint8_t blendIndex = 0;
};

void displayCharacter(uint8_t charNum, uint8_t position,
                     const DisplayOptions& opts = {});
```

**Files to modify:**

- `include/LED_Clock.h` - Add struct
- `src/LED_Clock.cpp` - Update function signature

**Reference:** Section 5.6 of code review report

______________________________________________________________________

### Add const Correctness

**Category:** Code Quality
**Effort:** Small
**Description:** Add const methods and return types where data is not modified.

**Examples:**

```cpp
const Config& ConfigManager::getConfig() const;  // Add const overload
const char* ConfigManager::getSchema() const;    // Return PROGMEM pointer
```

**Files to modify:**

- `include/ConfigManager.h`, `include/LED_Clock.h`, others

**Reference:** Section 5.7 of code review report

______________________________________________________________________

### Move DEBUG to Build Flags

**Category:** Development
**Effort:** Small
**Description:** Debug state currently controlled by commenting/uncommenting `#define DEBUG`. Move to build flags in platformio.ini.

**Implementation:**

```ini
[env:debug]
extends = env:esp32dev
build_flags =
  ${env:esp32dev.build_flags}
  -DDEBUG
```

**Files to modify:**

- `platformio.ini` - Add debug environment
- `include/config.h` - Remove commented #define

**Reference:** Section 6.4 of code review report

______________________________________________________________________

### Remove Unused Directories

**Category:** Cleanup
**Effort:** Trivial
**Description:** `lib/` directory is empty. Either use it or remove it.

**Implementation:**

- Remove `lib/` directory if not needed
- Or document its intended purpose in `lib/README`

**Reference:** Section 6.5 of code review report

______________________________________________________________________

## 📚 Documentation Improvements

### Create Advanced User Documentation

**Category:** Documentation
**Effort:** Medium
**Description:** Extend documentation for advanced users and integrators.

**Planned Additions:**

- **HARDWARE.md** - Wiring diagrams, LED mapping, assembly guide
- **CONFIGURATION.md** - Detailed walkthrough with web UI screenshots
- **DEVELOPMENT.md** - Environment setup, build process, debugging
- **TROUBLESHOOTING.md** - Extended troubleshooting guide beyond README
- **SECURITY.md** - Security best practices and vulnerability reporting

**Note:** Basic API and Architecture docs completed in Phase 7

______________________________________________________________________

### Add Visual Palette Previews

**Category:** Documentation
**Effort:** Small
**Description:** README lists palettes (Rainbow, Cloud, Lava, Ocean, Forest) but no visual previews. Add color swatches or screenshots.

**Files to modify:**

- README.md - Add palette preview images or color swatches

______________________________________________________________________

### Expand Cron Syntax Examples

**Category:** Documentation
**Effort:** Small
**Description:** README has basic cron examples but could provide more common use cases.

**Examples to add:**

- Every 15 minutes
- Every hour at :05 and :35
- Weekdays only
- Specific days of week

**Files to modify:**

- README.md - Expand cron section

______________________________________________________________________

## Completed Work (Phases 3-6)

These items were completed during the code review cleanup phases and are listed here for reference:

✅ **Phase 3 - Error Handling:**

- Fixed WiFi reconnection timing issues
- Added LittleFS error handling with fallback defaults
- Improved weather API error handling with WeatherStatus enum
- Added geolocation validation (coordinate ranges)

✅ **Phase 4 - Performance:**

- Cached palette lookups in ConfigManager
- Cached brightness time parsing
- Replaced String with const char\* in hot paths
- Added character mapping lookup table
- Cached cron schedule parsing
- Optimized color blending calculations
- Reduced JSON reallocations
- Optimized brightness fade transitions

✅ **Phase 5 - Refactoring:**

- Created SecureHTTPClient utility (eliminated HTTP duplication)
- Created ColorCalculator utility (centralized color logic)
- Created ConfigValidator utility (centralized validation)
- Refactored initWiFiManager() from 124 lines into 5 focused functions
- Split displayTemperature() into helper functions
- Removed dead code (updateBrightnessOld_REMOVED)
- Replaced magic numbers with WeatherStatus enum

✅ **Phase 6 - Cleanup:**

- Removed all trailing whitespace (112 instances)
- Standardized copyright headers (22 files)
- Verified all files have proper GPL v3 headers

______________________________________________________________________

## Notes

- This list is living and will be updated as work progresses
- Priorities may shift based on user feedback and discovered issues
- Some items may be combined or split during implementation
- Testing framework is the highest priority for long-term code quality
