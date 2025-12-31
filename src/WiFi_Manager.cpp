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

#include "config.h"
#include "Logger.h"
#include "WiFi_Manager.h"
#include "LED_Clock.h"
#include "ConfigStorage.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <ESP_DoubleResetDetector.h>
#include <ESPAsync_WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncDNSServer.h>
#include <ESP32Time.h>
#include <time.h>
#include <esp_task_wdt.h>

// Double Reset Detector
DoubleResetDetector* drd;

// WiFi Manager and web server components
AsyncWebServer* webServer;
AsyncDNSServer* dnsServer;
ESPAsync_WiFiManager* wifiManager;

// Timezone configuration
String timezoneString = "";
String timezoneNameString = "";

bool initialConfig = false;

// NTP servers
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const char* ntpServer3 = "time.google.com";

void configureNTP() {
  LOG_INFO("Configuring NTP...");
  LOG_DEBUGF("Timezone name: %s", timezoneNameString.length() > 0 ? timezoneNameString.c_str() : "EMPTY");
  LOG_DEBUGF("Timezone string: %s", timezoneString.length() > 0 ? timezoneString.c_str() : "EMPTY");
  if (timezoneString.length() > 0) {
    LOG_INFOF("Applying timezone: %s", timezoneString.c_str());
    configTzTime(timezoneString.c_str(), ntpServer1, ntpServer2, ntpServer3);
  } else {
    LOG_WARN("No timezone set, using UTC");
    configTime(0, 0, ntpServer1, ntpServer2, ntpServer3);
  }
  LOG_INFO("NTP configured");
}

// Helper functions for WiFi initialization
static bool initFileSystem() {
  if (FORMAT_FILESYSTEM) {
    LOG_WARN("Formatting filesystem...");
    LittleFS.format();
  }
  if (!LittleFS.begin(true)) {
    LOG_ERROR("LittleFS mount failed");
    return false;
  }
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  displayStatus(1);
  return true;
}

static void loadTimezoneConfig() {
  LOG_DEBUG("Loading saved timezone");
  ClockConfig clockConfig;
  if (loadClockConfig(clockConfig)) {
    if (strlen(clockConfig.TZ_Name) > 0) {
      timezoneNameString = String(clockConfig.TZ_Name);
      timezoneString = String(clockConfig.TZ);
      LOG_DEBUGF("Loaded from file - TZ_Name: %s, TZ: %s", timezoneNameString.c_str(), timezoneString.c_str());
    } else {
      LOG_DEBUG("Config file has no timezone");
    }
  } else {
    LOG_DEBUG("No saved config file found");
  }
}

static void setupWiFiManagerPortal() {
  webServer = new AsyncWebServer(HTTP_PORT);
  dnsServer = new AsyncDNSServer();
  wifiManager = new ESPAsync_WiFiManager(webServer, dnsServer, "LED-Clock");
  wifiManager->setDebugOutput(false);

  String storedSSID = wifiManager->WiFi_SSID();
  String storedPass = wifiManager->WiFi_Pass();

  if (storedSSID != "" && storedPass != "") {
    LOG_INFO("Found stored WiFi credentials");
  } else {
    LOG_INFO("No stored WiFi credentials found");
    initialConfig = true;
  }

  if (drd->detectDoubleReset()) {
    LOG_INFO("Double Reset Detected - entering config portal");
    initialConfig = true;
    wifiManager->setConfigPortalTimeout(0);
  } else {
    LOG_DEBUG("No Double Reset Detected");
    if (storedSSID != "" && storedPass != "") {
      wifiManager->setConfigPortalTimeout(15);
    }
  }
}

static bool handleConfigPortal() {
  displayStatus(2);
  String apSSID = portalSsid;
  String apPassword = portalPassword;
  LOG_INFOF("Starting config portal - SSID: %s, IP: 192.168.4.1", apSSID.c_str());

  if (!wifiManager->startConfigPortal(apSSID.c_str(), apPassword.c_str())) {
    LOG_WARN("Config portal timeout - restarting");
    delay(3000);
    ESP.restart();
  }

  timezoneNameString = wifiManager->getTimezoneName();
  LOG_DEBUGF("Timezone from portal: %s", timezoneNameString.length() > 0 ? timezoneNameString.c_str() : "EMPTY");

  if (timezoneNameString.length() > 0) {
    const char* tzResult = wifiManager->getTZ(timezoneNameString);
    if (tzResult != nullptr && strlen(tzResult) > 0) {
      timezoneString = String(tzResult);
      LOG_DEBUGF("Converted to POSIX TZ: %s", timezoneString.c_str());

      ClockConfig clockConfig;
      memset(&clockConfig, 0, sizeof(ClockConfig));
      strncpy(clockConfig.TZ_Name, timezoneNameString.c_str(), TZNAME_MAX_LEN - 1);
      strncpy(clockConfig.TZ, timezoneString.c_str(), TIMEZONE_MAX_LEN - 1);
      saveClockConfig(clockConfig);
    } else {
      LOG_ERROR("getTZ() returned null or empty");
    }
  }
  return true;
}

static bool connectToWiFi() {
  if (!initialConfig) {
    displayStatus(3);
    String apSSID = portalSsid;
    String apPassword = portalPassword;
    LOG_INFO("Connecting to saved WiFi...");
    if (!wifiManager->autoConnect(apSSID.c_str(), apPassword.c_str())) {
      LOG_ERROR("AutoConnect failed - restarting");
      delay(3000);
      ESP.restart();
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    LOG_INFOF("WiFi connected - SSID: %s, IP: %s", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    configureNTP();
    return true;
  }
  return false;
}

bool initWiFiManager() {
  if (!initFileSystem()) {
    return false;
  }
  loadTimezoneConfig();
  setupWiFiManagerPortal();
  if (initialConfig && !handleConfigPortal()) {
    return false;
  }
  return connectToWiFi();
}

// WiFi reconnection state tracking
static unsigned long lastReconnectAttempt = 0;
static unsigned long reconnectInterval = WIFI_RECONNECT_INTERVAL_MS;
static uint8_t reconnectAttempts = 0;
static bool wasConnected = true;
static unsigned long disconnectStartTime = 0;
static bool wifiEventHandlersRegistered = false;

void setupWiFiEventHandlers() {
  if (wifiEventHandlersRegistered) return;

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        LOG_WARN("WiFi disconnected event detected");
        if (wasConnected) {
          disconnectStartTime = millis();
        }
        wasConnected = false;
        break;
      case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        LOG_INFO("WiFi connected event detected");
        break;
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        LOG_INFOF("WiFi connected - IP: %s", WiFi.localIP().toString().c_str());
        reconnectAttempts = 0;
        reconnectInterval = WIFI_RECONNECT_INTERVAL_MS;
        wasConnected = true;
        disconnectStartTime = 0;
        break;
      default:
        break;
    }
  });

  wifiEventHandlersRegistered = true;
  LOG_DEBUG("WiFi event handlers registered");
}

bool checkWiFiStatus() {
  drd->loop();

  // Feed watchdog during WiFi recovery to prevent timeout
  esp_task_wdt_reset();

  // Setup event handlers on first call
  if (!wifiEventHandlersRegistered) {
    setupWiFiEventHandlers();
  }

  bool currentlyConnected = (WiFi.status() == WL_CONNECTED);
  bool justRecovered = false;

  // Detect reconnection
  if (currentlyConnected && !wasConnected) {
    LOG_INFO("WiFi connection recovered");
    wasConnected = true;
    reconnectAttempts = 0;
    reconnectInterval = WIFI_RECONNECT_INTERVAL_MS;
    disconnectStartTime = 0;
    justRecovered = true;
  }

  // Handle disconnection
  if (!currentlyConnected) {
    unsigned long now = millis();

    // Track when disconnection started
    if (wasConnected) {
      disconnectStartTime = now;
      wasConnected = false;
      LOG_WARN("WiFi connection lost - starting recovery");
      displayClockface("Er05");
    }

    // Check if it's time to attempt reconnection
    if (now - lastReconnectAttempt >= reconnectInterval) {
      lastReconnectAttempt = now;
      reconnectAttempts++;

      unsigned long disconnectedDuration = now - disconnectStartTime;

      // Full WiFi restart if disconnected too long or too many attempts
      if (disconnectedDuration >= WIFI_FULL_RESTART_THRESHOLD || reconnectAttempts >= WIFI_MAX_RECONNECT_ATTEMPTS) {
        LOG_WARNF("Performing full WiFi restart (attempt %d, disconnected for %lu ms)", reconnectAttempts, disconnectedDuration);

        // Feed watchdog before potentially long operation
        esp_task_wdt_reset();

        WiFi.disconnect(true);
        delay(100);
        WiFi.mode(WIFI_STA);
        WiFi.begin();

        // Feed watchdog after operation
        esp_task_wdt_reset();

        reconnectAttempts = 0;
        reconnectInterval = WIFI_RECONNECT_INTERVAL_MS;
      } else {
        LOG_DEBUGF("WiFi reconnect attempt %d/%d", reconnectAttempts, WIFI_MAX_RECONNECT_ATTEMPTS);
        WiFi.reconnect();

        // Exponential backoff
        reconnectInterval = min(reconnectInterval * 2, (unsigned long)WIFI_MAX_BACKOFF_MS);
      }
    }
  }

  return justRecovered;
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void syncRTCWithNTP(ESP32Time& rtc) {
  LOG_INFO("Syncing RTC with NTP...");
  struct tm timeinfo;
  const uint8_t maxRetries = 3;
  bool synced = false;

  for (uint8_t attempt = 1; attempt <= maxRetries && !synced; attempt++) {
    if (getLocalTime(&timeinfo, 10000)) {
      // Validate time is reasonable (after 2020-01-01)
      if (timeinfo.tm_year + 1900 >= 2020) {
        rtc.setTimeStruct(timeinfo);
        LOG_INFOF("RTC synced on attempt %d: %04d-%02d-%02d %02d:%02d:%02d",
                  attempt,
                  timeinfo.tm_year + 1900,
                  timeinfo.tm_mon + 1,
                  timeinfo.tm_mday,
                  timeinfo.tm_hour,
                  timeinfo.tm_min,
                  timeinfo.tm_sec);
        synced = true;
      } else {
        LOG_WARNF("NTP time invalid (year %d) on attempt %d, retrying...",
                  timeinfo.tm_year + 1900, attempt);
        delay(1000);
      }
    } else {
      LOG_WARNF("Failed to obtain time from NTP on attempt %d/%d",
                attempt, maxRetries);
      if (attempt < maxRetries) {
        delay(2000);
      }
    }
  }

  if (!synced) {
    LOG_ERROR("Failed to sync RTC with NTP after all retries");
  }
}
