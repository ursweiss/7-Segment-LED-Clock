# REST API Reference

This document provides basic reference for the web-based REST API endpoints. These endpoints are primarily used by the embedded web UI and are not intended for extensive external integration.

**Base URL:** `http://ledclock.local` (or device IP address)
**Authentication:** None (all endpoints are currently unauthenticated)
**Content-Type:** `application/json`

______________________________________________________________________

## Endpoints

### GET /

Serves the web configuration interface.

**Response:** HTML page

**Example:**

```bash
curl http://ledclock.local/
```

______________________________________________________________________

### GET /api/config

Get current device configuration.

**Response:** JSON object with all configuration settings

**Example:**

```bash
curl http://ledclock.local/api/config
```

**Response Example:**

```json
{
  "portalSsid": "LED-Clock-Setup",
  "ledBrightness": 200,
  "clockColorMode": 1,
  "weatherTempEnabled": true,
  "locationLatitude": "47.3769",
  "locationLongitude": "8.5417"
}
```

______________________________________________________________________

### POST /api/config

Update device configuration.

**Request Body:** JSON object with configuration fields to update (partial updates supported)

**Response:** Success or error message

**Example:**

```bash
curl -X POST http://ledclock.local/api/config \
  -H "Content-Type: application/json" \
  -d '{"ledBrightness": 150, "clockColorMode": 0}'
```

**Response Example:**

```json
{
  "success": true,
  "message": "Configuration saved"
}
```

**Error Responses:**

- `400` - Invalid JSON or validation error
- `413` - Payload too large (max 2048 bytes)
- `500` - Failed to save to filesystem

______________________________________________________________________

### GET /api/schema

Get the configuration schema used by the web UI.

**Response:** JSON schema describing all configuration fields, validation rules, and UI layout

**Example:**

```bash
curl http://ledclock.local/api/schema
```

______________________________________________________________________

### GET /api/version

Get device version and hardware information.

**Response:** JSON object with version info and chip details

**Example:**

```bash
curl http://ledclock.local/api/version
```

**Response Example:**

```json
{
  "version": "2025-12-28_2211",
  "chipModel": "ESP32-D0WDQ6",
  "chipCores": 2,
  "flashSize": 4194304,
  "freeHeap": 234567
}
```

______________________________________________________________________

### GET /api/geolocation

Detect current location based on IP address (uses ipapi.co service).

**Response:** JSON object with geographic coordinates and location details

**Example:**

```bash
curl http://ledclock.local/api/geolocation
```

**Response Example:**

```json
{
  "success": true,
  "latitude": "47.376900",
  "longitude": "8.541700",
  "city": "Zurich",
  "postalCode": "8001",
  "region": "Zurich",
  "country": "Switzerland"
}
```

**Error Responses:**

- `503` - WiFi not connected
- `500` - Geolocation lookup failed

**Privacy Note:** This endpoint sends a request to ipapi.co to determine location. No personal data is transmitted beyond the device's IP address.

______________________________________________________________________

### POST /api/restart

Restart the device.

**Response:** Success message (device restarts after 2 seconds)

**Example:**

```bash
curl -X POST http://ledclock.local/api/restart
```

**Response Example:**

```json
{
  "success": true,
  "message": "Device will restart in 2 seconds"
}
```

______________________________________________________________________

### POST /api/update

Upload firmware for OTA (Over-The-Air) update.

**Request:** Multipart form data with firmware binary file

**Response:** Success or error message (device restarts automatically on success)

**Example:**

```bash
curl -X POST http://ledclock.local/api/update \
  -F "file=@firmware.bin"
```

**Response Example:**

```json
{
  "success": true,
  "message": "Update complete. Restarting..."
}
```

**Error Responses:**

- `500` - Update failed (insufficient space, corrupted file, etc.)

**Note:** The device will restart automatically after a successful firmware update.

______________________________________________________________________

## Security Considerations

⚠️ **No Authentication:** All endpoints are currently unauthenticated. Anyone on the same network can access and modify the configuration.

⚠️ **No TLS:** Communication is unencrypted HTTP. Configuration data (including WiFi portal credentials) is transmitted in plain text.

**Recommendations:**

- Use the device only on trusted networks
- Change default portal password from the fallback
- Consider network-level security (firewall, VLAN isolation)

______________________________________________________________________

## Rate Limiting

No rate limiting is currently implemented. The device may become unresponsive if endpoints are called too frequently.

______________________________________________________________________

## Notes

- All configuration changes are persisted to LittleFS filesystem
- Some configuration changes require a device restart to take effect (noted in web UI)
- The device uses mDNS for hostname resolution (`ledclock.local`)
- API responses may include additional fields not documented here
