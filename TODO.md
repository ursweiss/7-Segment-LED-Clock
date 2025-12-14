# TODO List

## Security & Network

### Add Proper TLS Certificate Validation for Geolocation API

**Priority:** Medium
**Status:** Open
**Description:** Currently using `client.setInsecure()` for HTTPS connections to ipapi.co geolocation service and open-meto.com weather service. This bypasses certificate validation for pragmatic reasons but should be improved.

**Implementation Options:**

1. **Embed Root CA Certificate** (Recommended)

   - Add GTS Root R4 certificate (Google Trust Services) to WebConfig.cpp
   - Use `client.setCACert(rootCA)` instead of `client.setInsecure()`
   - Certificate valid until: January 28, 2028
   - Increases flash usage by ~1-2KB

1. **Use ESP32 Certificate Store**

   - More flexible but more complex implementation
   - Allows multiple certificates without code changes

**Files to modify:**

- `src/WebConfig.cpp` - Add certificate and use `setCACert()` in geolocation endpoint

**Note:** ESP32 Arduino framework does NOT bundle CA certificates by default. Each HTTPS connection requires explicit certificate configuration.

______________________________________________________________________

## Debugging & User Support

### Add Debug Output Toggle to Web UI

**Priority:** Medium
**Status:** Open
**Description:** Currently, debug output is controlled by `#define DEBUG` in `include/config.h`, which requires recompilation to enable/disable. Add a runtime toggle in the web configuration interface so users can enable debug output without recompiling.

**Implementation Plan:**

1. Add `debugEnabled` boolean field to Config struct in `include/ConfigManager.h`
1. Update `include/schema.h` to add debug toggle to "Advanced" settings group
1. Modify `Logger.h` to check runtime config instead of just compile-time `#define DEBUG`
1. Add logic to persist debug setting to LittleFS like other config options
1. Update `README.md` to document the debug feature

**Benefits:**

- Users can enable debug output to help troubleshoot issues
- No need to recompile firmware for debugging
- Debug logs can be captured via serial monitor during troubleshooting
- Useful for reporting bugs with detailed information

**Files to modify:**

- `include/ConfigManager.h` - Add debugEnabled field
- `include/schema.h` - Add UI field for debug toggle
- `include/Logger.h` - Update macros to check runtime config
- `src/ConfigManager.cpp` - Handle new config field
- `README.md` - Document debug feature

**Notes:**

- Keep existing `#define DEBUG` as a build-time option (for developers)
- Runtime config should extend/override compile-time setting
- Consider adding a debug output buffer/log file to avoid excessive serial output

______________________________________________________________________

## Display Features

### Add STATIC_PALETTE Color Mode

**Priority:** Low
**Status:** Open
**Description:** Implement static palette mode (mode 2) for clock color display.

**Current Implementation:**

- Mode 0: SOLID - Single solid color
- Mode 1: PALETTE - Animated rainbow/palette effects with per-character blending
- Mode 2: STATIC_PALETTE - *Not yet implemented*

**Details:**

- Mentioned in `include/config.h` line 13
- Comment: `(2 => STATIC_PALETTE - To-Do)`

**Implementation Notes:**

- Add new case to color mode switch in `LED_Clock.cpp`
- Define static palette color progression
- Update web UI schema in `include/schema.h`
- Update README.md with new mode documentation

______________________________________________________________________

## Documentation

### Update README Future Enhancements Section

**Priority:** Low
**Status:** Open
**Description:** The "Future Enhancements" section in README.md mentions features that may already be implemented or need clarification.

**Current list:**

- Additional display modes and animations
- Weather forecast display
- Multi-timezone support

**Action:** Review and update based on current implementation status.

______________________________________________________________________

## Completed

*(No completed items yet)*
