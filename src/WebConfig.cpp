/*
 * This file is part of the 7 Segment LED Clock Project
 *   https://github.com/ursweiss/7-Segment-LED-Clock
 *   https://www.printables.com/model/68013-7-segment-led-clock
 *
 * Copyright (c) 2021-2025 Urs Weiss
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"  // Must be first to get DEBUG definition
#include "WebConfig.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "WiFi_Manager.h"
#include "web_html.h"
#include "version.h"
#include "CronHelper.h"
#include "BrightnessControl.h"
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

static bool restartRequested = false;
static unsigned long restartRequestTime = 0;

bool startMDNS(const char* hostname) {
  if (!MDNS.begin(hostname)) {
    LOG_ERROR("Failed to start mDNS");
    return false;
  }

  MDNS.addService("http", "tcp", 80);
  LOG_INFOF("mDNS started: http://%s.local", hostname);
  return true;
}

bool initWebConfig(AsyncWebServer* server) {
  if (!server) {
    LOG_ERROR("Web server is null");
    return false;
  }

  // Test route
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", INDEX_HTML);
  });

  // Get current configuration
  server->on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    Config& cfg = configManager.getConfig();

    StaticJsonDocument<2048> doc;
    doc["portalSsid"] = cfg.portalSsid;
    doc["portalPassword"] = cfg.portalPassword;
    doc["clockColorMode"] = cfg.clockColorMode;
    doc["clockColorSolid"] = (uint32_t)cfg.clockColorSolid;
    doc["clockColorPaletteIndex"] = cfg.clockColorPaletteIndex;
    doc["clockColorCharBlend"] = cfg.clockColorCharBlend;
    doc["clockColorBlending"] = cfg.clockColorBlending;
    doc["clockSecIndicatorDiff"] = cfg.clockSecIndicatorDiff;
    doc["locationLatitude"] = cfg.locationLatitude;
    doc["locationLongitude"] = cfg.locationLongitude;
    doc["locationUnits"] = cfg.locationUnits;
    doc["weatherTempEnabled"] = cfg.weatherTempEnabled;
    doc["weatherTempDisplayTime"] = cfg.weatherTempDisplayTime;
    doc["weatherTempMin"] = cfg.weatherTempMin;
    doc["weatherTempMax"] = cfg.weatherTempMax;
    doc["weatherTempSchedule"] = cfg.weatherTempSchedule;
    doc["weatherUpdateSchedule"] = cfg.weatherUpdateSchedule;
    doc["ledBrightness"] = cfg.ledBrightness;
    doc["ledDimEnabled"] = cfg.ledDimEnabled;
    doc["ledDimBrightness"] = cfg.ledDimBrightness;
    doc["ledDimFadeDuration"] = cfg.ledDimFadeDuration;
    doc["ledDimStartTime"] = cfg.ledDimStartTime;
    doc["ledDimEndTime"] = cfg.ledDimEndTime;
    doc["clockUpdateSchedule"] = cfg.clockUpdateSchedule;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Get schema
  server->on("/api/schema", HTTP_GET, [](AsyncWebServerRequest *request) {
    String schema = configManager.getSchema();
    request->send(200, "application/json", schema);
  });

  // Get version info
  server->on("/api/version", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<256> doc;
    doc["version"] = getBuildVersion();
    doc["chipModel"] = ESP.getChipModel();
    doc["chipCores"] = ESP.getChipCores();
    doc["flashSize"] = ESP.getFlashChipSize();
    doc["freeHeap"] = ESP.getFreeHeap();

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Geolocation lookup
  server->on("/api/geolocation", HTTP_GET, [](AsyncWebServerRequest *request) {
    #ifdef DEBUG
    LOG_DEBUG("Geolocation lookup requested");
    #endif

    // Check WiFi connection before attempting request
    if (!isWiFiConnected()) {
      LOG_WARN("Geolocation lookup skipped - WiFi not connected");
      request->send(503, "application/json", "{\"success\":false,\"error\":\"WiFi not connected\"}");
      return;
    }

    // Use do-while(false) pattern for guaranteed cleanup
    WiFiClientSecure* client = nullptr;
    HTTPClient* http = nullptr;
    bool success = false;
    String responseStr;

    do {
      client = new WiFiClientSecure();
      if (!client) {
        LOG_ERROR("Failed to allocate WiFiClientSecure");
        break;
      }
      client->setInsecure();

      http = new HTTPClient();
      if (!http) {
        LOG_ERROR("Failed to allocate HTTPClient");
        break;
      }
      http->setTimeout(5000);
      http->setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

      if (!http->begin(*client, "https://ipapi.co/json/")) {
        LOG_ERROR("Failed to initialize HTTPS connection");
        break;
      }

      int httpCode = http->GET();
      if (httpCode != HTTP_CODE_OK) {
        LOG_ERRORF("Geolocation request failed with code %d", httpCode);
        break;
      }

      String payload = http->getString();
      if (payload.length() == 0) {
        LOG_ERROR("Empty response");
        break;
      }

      #ifdef DEBUG
      LOG_DEBUGF("Geolocation response: %.100s%s", payload.c_str(), payload.length() > 100 ? "..." : "");
      #endif

      // Parse JSON
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        LOG_ERRORF("JSON parsing failed: %s", error.c_str());
        break;
      }

      // Extract and validate location data
      if (!doc.containsKey("latitude") || !doc.containsKey("longitude")) {
        LOG_ERROR("Missing location data in response");
        break;
      }

      float lat = doc["latitude"].as<float>();
      float lon = doc["longitude"].as<float>();

      // Validate coordinate ranges
      if (lat < -90.0f || lat > 90.0f || lon < -180.0f || lon > 180.0f) {
        LOG_ERRORF("Invalid coordinates: lat=%.6f, lon=%.6f", lat, lon);
        break;
      }

      // Build response
      StaticJsonDocument<512> response;
      response["success"] = true;
      response["latitude"] = String(lat, 6);
      response["longitude"] = String(lon, 6);
      response["city"] = doc["city"].as<String>();
      response["postalCode"] = doc["postal"].as<String>();
      response["region"] = doc["region"].as<String>();
      response["country"] = doc["country_name"].as<String>();

      serializeJson(response, responseStr);

      #ifdef DEBUG
      LOG_DEBUGF("Geolocation: %s, %s (%.6f, %.6f)",
        response["city"].as<const char*>(),
        response["country"].as<const char*>(),
        lat, lon);
      #endif

      success = true;
    } while (false);

    // Guaranteed cleanup
    if (http) {
      http->end();
      delete http;
    }
    if (client) {
      delete client;
    }

    if (success) {
      request->send(200, "application/json", responseStr);
    } else {
      request->send(500, "application/json", "{\"success\":false,\"error\":\"Geolocation lookup failed\"}");
    }
  });

  // Update configuration
  server->on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      // Validate total payload size before parsing
      const size_t MAX_CONFIG_SIZE = 2048;

      if (total > MAX_CONFIG_SIZE) {
        LOG_ERRORF("Config payload too large: %d bytes (max %d)", total, MAX_CONFIG_SIZE);
        request->send(413, "application/json",
          "{\"error\":\"Request payload too large\"}");
        return;
      }

      StaticJsonDocument<2048> doc;
      DeserializationError error = deserializeJson(doc, data, len);

      if (error) {
        LOG_ERRORF("Failed to parse config JSON: %s", error.c_str());
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      Config& cfg = configManager.getConfig();

      // Validate and update config values from JSON with bounds checking

      // WiFi settings - validate string lengths
      if (doc.containsKey("portalSsid")) {
        String ssid = doc["portalSsid"].as<String>();
        if (ssid.length() == 0 || ssid.length() > 32) {
          request->send(400, "application/json",
            "{\"error\":\"portalSsid must be 1-32 characters\"}");
          return;
        }
        cfg.portalSsid = ssid;
      }

      if (doc.containsKey("portalPassword")) {
        String password = doc["portalPassword"].as<String>();
        if (password.length() < 8 || password.length() > 63) {
          request->send(400, "application/json",
            "{\"error\":\"portalPassword must be 8-63 characters\"}");
          return;
        }
        cfg.portalPassword = password;
      }

      // Clock settings - validate numeric ranges
      if (doc.containsKey("clockColorMode")) {
        uint8_t mode = doc["clockColorMode"];
        if (mode > 2) {
          request->send(400, "application/json",
            "{\"error\":\"clockColorMode must be 0-2\"}");
          return;
        }
        cfg.clockColorMode = mode;
      }

      if (doc.containsKey("clockColorSolid")) cfg.clockColorSolid = doc["clockColorSolid"].as<uint32_t>();

      if (doc.containsKey("clockColorPaletteIndex")) {
        uint8_t paletteIdx = doc["clockColorPaletteIndex"];
        if (paletteIdx > 4) {
          request->send(400, "application/json",
            "{\"error\":\"clockColorPaletteIndex must be 0-4\"}");
          return;
        }
        cfg.clockColorPaletteIndex = paletteIdx;
      }

      if (doc.containsKey("clockColorCharBlend")) cfg.clockColorCharBlend = doc["clockColorCharBlend"];

      if (doc.containsKey("clockColorBlending")) {
        uint8_t blending = doc["clockColorBlending"];
        if (blending > 1) {
          request->send(400, "application/json",
            "{\"error\":\"clockColorBlending must be 0 or 1\"}");
          return;
        }
        cfg.clockColorBlending = blending;
      }

      if (doc.containsKey("clockSecIndicatorDiff")) cfg.clockSecIndicatorDiff = doc["clockSecIndicatorDiff"];

      // Weather settings - validate coordinates
      if (doc.containsKey("locationLatitude")) {
        String lat = doc["locationLatitude"].as<String>();
        if (lat.length() > 20) {
          request->send(400, "application/json",
            "{\"error\":\"locationLatitude too long\"}");
          return;
        }
        cfg.locationLatitude = lat;
      }

      if (doc.containsKey("locationLongitude")) {
        String lon = doc["locationLongitude"].as<String>();
        if (lon.length() > 20) {
          request->send(400, "application/json",
            "{\"error\":\"locationLongitude too long\"}");
          return;
        }
        cfg.locationLongitude = lon;
      }

      if (doc.containsKey("locationUnits")) {
        String units = doc["locationUnits"].as<String>();
        if (units != "metric" && units != "imperial") {
          request->send(400, "application/json",
            "{\"error\":\"locationUnits must be 'metric' or 'imperial'\"}");
          return;
        }
        cfg.locationUnits = units;
      }

      if (doc.containsKey("weatherTempEnabled")) {
        uint8_t enabled = doc["weatherTempEnabled"];
        if (enabled > 1) {
          request->send(400, "application/json",
            "{\"error\":\"weatherTempEnabled must be 0 or 1\"}");
          return;
        }
        cfg.weatherTempEnabled = enabled;
      }

      if (doc.containsKey("weatherTempDisplayTime")) {
        uint8_t displayTime = doc["weatherTempDisplayTime"];
        if (displayTime == 0 || displayTime > 60) {
          request->send(400, "application/json",
            "{\"error\":\"weatherTempDisplayTime must be 1-60 seconds\"}");
          return;
        }
        cfg.weatherTempDisplayTime = displayTime;
      }

      if (doc.containsKey("weatherTempMin")) {
        int8_t tempMin = doc["weatherTempMin"];
        if (tempMin < -99 || tempMin > 98) {
          request->send(400, "application/json",
            "{\"error\":\"weatherTempMin must be -99 to 98\"}");
          return;
        }
        cfg.weatherTempMin = tempMin;
      }

      if (doc.containsKey("weatherTempMax")) {
        int8_t tempMax = doc["weatherTempMax"];
        if (tempMax < -98 || tempMax > 99) {
          request->send(400, "application/json",
            "{\"error\":\"weatherTempMax must be -98 to 99\"}");
          return;
        }
        cfg.weatherTempMax = tempMax;
      }

      // Validate cron expressions
      if (doc.containsKey("weatherTempSchedule")) {
        String cronStr = doc["weatherTempSchedule"].as<String>();
        if (cronStr.length() > 50) {
          request->send(400, "application/json",
            "{\"error\":\"weatherTempSchedule too long\"}");
          return;
        }
        if (!CronHelper::validateCron(cronStr.c_str())) {
          request->send(400, "application/json",
            "{\"error\":\"Invalid cron expression in weatherTempSchedule\"}");
          return;
        }
        cfg.weatherTempSchedule = cronStr;
      }

      if (doc.containsKey("weatherUpdateSchedule")) {
        String cronStr = doc["weatherUpdateSchedule"].as<String>();
        if (cronStr.length() > 50) {
          request->send(400, "application/json",
            "{\"error\":\"weatherUpdateSchedule too long\"}");
          return;
        }
        if (!CronHelper::validateCron(cronStr.c_str())) {
          request->send(400, "application/json",
            "{\"error\":\"Invalid cron expression in weatherUpdateSchedule\"}");
          return;
        }
        cfg.weatherUpdateSchedule = cronStr;
      }

      // LED settings - validate ranges
      if (doc.containsKey("ledBrightness")) cfg.ledBrightness = doc["ledBrightness"];

      if (doc.containsKey("ledDimEnabled")) {
        uint8_t enabled = doc["ledDimEnabled"];
        if (enabled > 1) {
          request->send(400, "application/json",
            "{\"error\":\"ledDimEnabled must be 0 or 1\"}");
          return;
        }
        cfg.ledDimEnabled = enabled;
      }

      if (doc.containsKey("ledDimBrightness")) cfg.ledDimBrightness = doc["ledDimBrightness"];

      if (doc.containsKey("ledDimFadeDuration")) {
        uint8_t fadeDuration = doc["ledDimFadeDuration"];
        if (fadeDuration == 0 || fadeDuration > 120) {
          request->send(400, "application/json",
            "{\"error\":\"ledDimFadeDuration must be 1-120 seconds\"}");
          return;
        }
        cfg.ledDimFadeDuration = fadeDuration;
      }

      // Validate time format (HH:MM)
      if (doc.containsKey("ledDimStartTime")) {
        String timeStr = doc["ledDimStartTime"].as<String>();
        int hours, minutes;
        if (!parseTime(timeStr.c_str(), hours, minutes) ||
            hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
          request->send(400, "application/json",
            "{\"error\":\"Invalid time format in ledDimStartTime (use HH:MM)\"}");
          return;
        }
        cfg.ledDimStartTime = timeStr;
      }

      if (doc.containsKey("ledDimEndTime")) {
        String timeStr = doc["ledDimEndTime"].as<String>();
        int hours, minutes;
        if (!parseTime(timeStr.c_str(), hours, minutes) ||
            hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
          request->send(400, "application/json",
            "{\"error\":\"Invalid time format in ledDimEndTime (use HH:MM)\"}");
          return;
        }
        cfg.ledDimEndTime = timeStr;
      }

      // Validate clock update schedule
      if (doc.containsKey("clockUpdateSchedule")) {
        String cronStr = doc["clockUpdateSchedule"].as<String>();
        if (cronStr.length() > 50) {
          request->send(400, "application/json",
            "{\"error\":\"clockUpdateSchedule too long\"}");
          return;
        }
        if (!CronHelper::validateCron(cronStr.c_str())) {
          request->send(400, "application/json",
            "{\"error\":\"Invalid cron expression in clockUpdateSchedule\"}");
          return;
        }
        cfg.clockUpdateSchedule = cronStr;
      }

      // Save to LittleFS
      if (configManager.saveConfig()) {
        LOG_INFO("Configuration saved successfully");
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Configuration saved\"}");
      } else {
        LOG_ERROR("Failed to save configuration");
        request->send(500, "application/json", "{\"error\":\"Failed to save configuration\"}");
      }
    });

  // Restart device
  server->on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    LOG_WARN("Restart requested via API");
    restartRequested = true;
    restartRequestTime = millis();
    request->send(200, "application/json", "{\"success\":true,\"message\":\"Device will restart in 2 seconds\"}");
  });

  // OTA Update endpoint
  server->on("/api/update", HTTP_POST,
    [](AsyncWebServerRequest *request) {
      // This is the response handler after upload completes
      bool success = !Update.hasError();
      if (success) {
        LOG_INFO("OTA Update successful");
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Update complete. Restarting...\"}");
        restartRequested = true;
        restartRequestTime = millis();
      } else {
        LOG_ERROR("OTA Update failed");
        request->send(500, "application/json", "{\"success\":false,\"message\":\"Update failed\"}");
      }
    },
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      // This is the upload handler
      if (!index) {
        LOG_INFOF("OTA Update started: %s", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
          LOG_ERROR("Failed to begin OTA update");
          Update.printError(Serial);
        }
      }

      if (len) {
        if (Update.write(data, len) != len) {
          LOG_ERROR("Failed to write OTA data");
          Update.printError(Serial);
        }
      }

      if (final) {
        if (Update.end(true)) {
          LOG_INFOF("OTA Update finished: %u bytes", index + len);
        } else {
          LOG_ERROR("OTA Update end failed");
          Update.printError(Serial);
        }
      }
    }
  );

  LOG_INFO("Web config routes initialized");
  return true;
}

bool isRestartRequested() {
  if (restartRequested && (millis() - restartRequestTime >= 2000)) {
    return true;
  }
  return false;
}
