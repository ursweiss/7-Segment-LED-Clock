#include "config.h"  // Must be first to get DEBUG definition
#include "WebConfig.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "WiFi_Manager.h"
#include "web_html.h"
#include "version.h"
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

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(5000);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

    if (!http.begin(client, "https://ipapi.co/json/")) {
      LOG_ERROR("Failed to initialize HTTPS connection");
      request->send(500, "application/json", "{\"success\":false,\"error\":\"Connection initialization failed\"}");
      return;
    }

    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
      LOG_ERRORF("Geolocation request failed with code %d", httpCode);
      http.end();
      request->send(500, "application/json", "{\"success\":false,\"error\":\"HTTP request failed\"}");
      return;
    }

    String payload = http.getString();
    http.end();

    if (payload.length() == 0) {
      LOG_ERROR("Empty response");
      request->send(500, "application/json", "{\"success\":false,\"error\":\"Empty response\"}");
      return;
    }

    // Parse JSON
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      LOG_ERRORF("JSON parsing failed: %s", error.c_str());
      request->send(500, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
      return;
    }

    // Extract location data (ipapi.co format)
    if (!doc.containsKey("latitude") || !doc.containsKey("longitude")) {
      LOG_ERROR("Missing location data in response");
      request->send(500, "application/json", "{\"success\":false,\"error\":\"Invalid response structure\"}");
      return;
    }

    // Build response
    StaticJsonDocument<512> response;
    response["success"] = true;
    response["latitude"] = String(doc["latitude"].as<float>(), 6);
    response["longitude"] = String(doc["longitude"].as<float>(), 6);
    response["city"] = doc["city"].as<String>();
    response["postalCode"] = doc["postal"].as<String>();
    response["region"] = doc["region"].as<String>();
    response["country"] = doc["country_name"].as<String>();

    String responseStr;
    serializeJson(response, responseStr);

    #ifdef DEBUG
    LOG_DEBUGF("Geolocation: %s, %s (%s, %s)",
      response["city"].as<const char*>(),
      response["country"].as<const char*>(),
      response["latitude"].as<const char*>(),
      response["longitude"].as<const char*>());
    #endif

    request->send(200, "application/json", responseStr);
  });

  // Update configuration
  server->on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      StaticJsonDocument<2048> doc;
      DeserializationError error = deserializeJson(doc, data, len);

      if (error) {
        LOG_ERROR("Failed to parse config JSON");
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      Config& cfg = configManager.getConfig();

      // Update config values from JSON
      if (doc.containsKey("portalSsid")) cfg.portalSsid = doc["portalSsid"].as<String>();
      if (doc.containsKey("portalPassword")) cfg.portalPassword = doc["portalPassword"].as<String>();
      if (doc.containsKey("clockColorMode")) cfg.clockColorMode = doc["clockColorMode"];
      if (doc.containsKey("clockColorSolid")) cfg.clockColorSolid = doc["clockColorSolid"].as<uint32_t>();
      if (doc.containsKey("clockColorPaletteIndex")) cfg.clockColorPaletteIndex = doc["clockColorPaletteIndex"];
      if (doc.containsKey("clockColorCharBlend")) cfg.clockColorCharBlend = doc["clockColorCharBlend"];
      if (doc.containsKey("clockColorBlending")) cfg.clockColorBlending = doc["clockColorBlending"];
      if (doc.containsKey("clockSecIndicatorDiff")) cfg.clockSecIndicatorDiff = doc["clockSecIndicatorDiff"];
      if (doc.containsKey("locationLatitude")) cfg.locationLatitude = doc["locationLatitude"].as<String>();
      if (doc.containsKey("locationLongitude")) cfg.locationLongitude = doc["locationLongitude"].as<String>();
      if (doc.containsKey("locationUnits")) cfg.locationUnits = doc["locationUnits"].as<String>();
      if (doc.containsKey("weatherTempEnabled")) cfg.weatherTempEnabled = doc["weatherTempEnabled"];
      if (doc.containsKey("weatherTempDisplayTime")) cfg.weatherTempDisplayTime = doc["weatherTempDisplayTime"];
      if (doc.containsKey("weatherTempMin")) cfg.weatherTempMin = doc["weatherTempMin"];
      if (doc.containsKey("weatherTempMax")) cfg.weatherTempMax = doc["weatherTempMax"];
      if (doc.containsKey("weatherTempSchedule")) cfg.weatherTempSchedule = doc["weatherTempSchedule"].as<String>();
      if (doc.containsKey("weatherUpdateSchedule")) cfg.weatherUpdateSchedule = doc["weatherUpdateSchedule"].as<String>();
      if (doc.containsKey("ledBrightness")) cfg.ledBrightness = doc["ledBrightness"];
      if (doc.containsKey("ledDimEnabled")) cfg.ledDimEnabled = doc["ledDimEnabled"];
      if (doc.containsKey("ledDimBrightness")) cfg.ledDimBrightness = doc["ledDimBrightness"];
      if (doc.containsKey("ledDimFadeDuration")) cfg.ledDimFadeDuration = doc["ledDimFadeDuration"];
      if (doc.containsKey("ledDimStartTime")) cfg.ledDimStartTime = doc["ledDimStartTime"].as<String>();
      if (doc.containsKey("ledDimEndTime")) cfg.ledDimEndTime = doc["ledDimEndTime"].as<String>();
      if (doc.containsKey("clockUpdateSchedule")) cfg.clockUpdateSchedule = doc["clockUpdateSchedule"].as<String>();

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
