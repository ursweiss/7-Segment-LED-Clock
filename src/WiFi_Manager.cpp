/*
 * This file is part of the 7 Segment LED Clock Project
 * (https://www.prusaprinters.org/prints/68013-7-segment-led-clock).
 *
 * Copyright (c) 2021 Urs Weiss.
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

bool initWiFiManager() {
  if (FORMAT_FILESYSTEM) {
    LOG_WARN("Formatting filesystem...");
    LittleFS.format();
  }
  if (!LittleFS.begin(true)) {
    LOG_ERROR("LittleFS mount failed");
    return false;
  }
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  LOG_INFO("Initialize");
  displayStatus(1);
  delay(1000);
  webServer = new AsyncWebServer(HTTP_PORT);
  dnsServer = new AsyncDNSServer();
  wifiManager = new ESPAsync_WiFiManager(webServer, dnsServer, "LED-Clock");
  wifiManager->setDebugOutput(false);
  String apSSID = portalSsid;
  String apPassword = portalPassword;
  String storedSSID = wifiManager->WiFi_SSID();
  String storedPass = wifiManager->WiFi_Pass();
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
  if (storedSSID != "" && storedPass != "") {
    LOG_INFO("Found stored WiFi credentials");
  } else {
    LOG_INFO("No stored WiFi credentials found");
  }
  if ((storedSSID == "") || (storedPass == "")) {
    LOG_WARN("No stored credentials - entering config portal");
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
  if (initialConfig) {
    displayStatus(2);
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
  } else {
    displayStatus(3);
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

void checkWiFiStatus() {
  drd->loop();
  if (WiFi.status() != WL_CONNECTED) {
    LOG_DEBUG("WiFi disconnected, attempting reconnect...");
    WiFi.reconnect();
  }
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void syncRTCWithNTP(ESP32Time& rtc) {
  LOG_INFO("Syncing RTC with NTP...");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 10000)) {
    rtc.setTimeStruct(timeinfo);
    LOG_INFOF("RTC synced: %04d-%02d-%02d %02d:%02d:%02d",
              timeinfo.tm_year + 1900,
              timeinfo.tm_mon + 1,
              timeinfo.tm_mday,
              timeinfo.tm_hour,
              timeinfo.tm_min,
              timeinfo.tm_sec);
  } else {
    LOG_ERROR("NTP sync failed - timeout");
  }
}
