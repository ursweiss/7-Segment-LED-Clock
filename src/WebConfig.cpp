#include "WebConfig.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "web_html.h"
#include "version.h"
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <Update.h>

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
    doc["owmApiServer"] = cfg.owmApiServer;
    doc["owmApiKey"] = cfg.owmApiKey;
    doc["owmLocation"] = cfg.owmLocation;
    doc["owmUnits"] = cfg.owmUnits;
    doc["owmTempEnabled"] = cfg.owmTempEnabled;
    doc["owmTempDisplayTime"] = cfg.owmTempDisplayTime;
    doc["owmTempMin"] = cfg.owmTempMin;
    doc["owmTempMax"] = cfg.owmTempMax;
    doc["owmTempSchedule"] = cfg.owmTempSchedule;
    doc["owmUpdateSchedule"] = cfg.owmUpdateSchedule;
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
      if (doc.containsKey("owmApiServer")) cfg.owmApiServer = doc["owmApiServer"].as<String>();
      if (doc.containsKey("owmApiKey")) cfg.owmApiKey = doc["owmApiKey"].as<String>();
      if (doc.containsKey("owmLocation")) cfg.owmLocation = doc["owmLocation"].as<String>();
      if (doc.containsKey("owmUnits")) cfg.owmUnits = doc["owmUnits"].as<String>();
      if (doc.containsKey("owmTempEnabled")) cfg.owmTempEnabled = doc["owmTempEnabled"];
      if (doc.containsKey("owmTempDisplayTime")) cfg.owmTempDisplayTime = doc["owmTempDisplayTime"];
      if (doc.containsKey("owmTempMin")) cfg.owmTempMin = doc["owmTempMin"];
      if (doc.containsKey("owmTempMax")) cfg.owmTempMax = doc["owmTempMax"];
      if (doc.containsKey("owmTempSchedule")) cfg.owmTempSchedule = doc["owmTempSchedule"].as<String>();
      if (doc.containsKey("owmUpdateSchedule")) cfg.owmUpdateSchedule = doc["owmUpdateSchedule"].as<String>();
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
