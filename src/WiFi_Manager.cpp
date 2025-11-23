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
  Serial.println("=== Configuring NTP ===");
  Serial.print("Timezone name: ");
  Serial.println(timezoneNameString.length() > 0 ? timezoneNameString : "EMPTY");
  Serial.print("Timezone string: ");
  Serial.println(timezoneString.length() > 0 ? timezoneString : "EMPTY");
  if (timezoneString.length() > 0) {
    Serial.print("Applying timezone with configTzTime: ");
    Serial.println(timezoneString);
    configTzTime(timezoneString.c_str(), ntpServer1, ntpServer2, ntpServer3);
  } else {
    Serial.println("WARNING: No timezone set, using UTC");
    configTime(0, 0, ntpServer1, ntpServer2, ntpServer3);
  }
  Serial.println("=== NTP configured ===");
}

bool initWiFiManager() {
  #ifdef DEBUG
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n7-Segment LED Clock Starting...");
  #endif
  if (FORMAT_FILESYSTEM) {
    #ifdef DEBUG
    Serial.println("Formatting filesystem...");
    #endif
    LittleFS.format();
  }
  if (!LittleFS.begin(true)) {
    #ifdef DEBUG
    Serial.println("LittleFS mount failed");
    #endif
    return false;
  }
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
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
  Serial.println("=== Loading saved timezone ===");
  ClockConfig clockConfig;
  if (loadClockConfig(clockConfig)) {
    if (strlen(clockConfig.TZ_Name) > 0) {
      timezoneNameString = String(clockConfig.TZ_Name);
      timezoneString = String(clockConfig.TZ);
      Serial.print("Loaded from file - TZ_Name: ");
      Serial.print(timezoneNameString);
      Serial.print(", TZ: ");
      Serial.println(timezoneString);
    } else {
      Serial.println("Config file has no timezone");
    }
  } else {
    Serial.println("No saved config file found");
  }
  Serial.println("=== Timezone load complete ===");
  #ifdef DEBUG
  if (storedSSID != "" && storedPass != "") {
    Serial.println("Found stored WiFi credentials");
  } else {
    Serial.println("No stored WiFi credentials found");
  }
  #endif
  if ((storedSSID == "") || (storedPass == "")) {
    #ifdef DEBUG
    Serial.println("No stored credentials - entering config portal");
    #endif
    initialConfig = true;
  }
  if (drd->detectDoubleReset()) {
    #ifdef DEBUG
    Serial.println("Double Reset Detected - entering config portal");
    #endif
    initialConfig = true;
    wifiManager->setConfigPortalTimeout(0);
  } else {
    #ifdef DEBUG
    Serial.println("No Double Reset Detected");
    #endif
    if (storedSSID != "" && storedPass != "") {
      wifiManager->setConfigPortalTimeout(15);
    }
  }
  if (initialConfig) {
    displayStatus(2);
    #ifdef DEBUG
    Serial.print("Starting config portal - SSID: ");
    Serial.print(apSSID);
    Serial.print(", IP: 192.168.4.1");
    #endif
    if (!wifiManager->startConfigPortal(apSSID.c_str(), apPassword.c_str())) {
      #ifdef DEBUG
      Serial.println("\nConfig portal timeout - restarting");
      #endif
      delay(3000);
      ESP.restart();
    }
    Serial.println("=== Config portal completed ===");
    timezoneNameString = wifiManager->getTimezoneName();
    Serial.print("Timezone from portal: ");
    Serial.println(timezoneNameString.length() > 0 ? timezoneNameString : "EMPTY");
    if (timezoneNameString.length() > 0) {
      const char* tzResult = wifiManager->getTZ(timezoneNameString);
      if (tzResult != nullptr && strlen(tzResult) > 0) {
        timezoneString = String(tzResult);
        Serial.print("Converted to POSIX TZ: ");
        Serial.println(timezoneString);
        ClockConfig clockConfig;
        memset(&clockConfig, 0, sizeof(ClockConfig));
        strncpy(clockConfig.TZ_Name, timezoneNameString.c_str(), TZNAME_MAX_LEN - 1);
        strncpy(clockConfig.TZ, timezoneString.c_str(), TIMEZONE_MAX_LEN - 1);
        saveClockConfig(clockConfig);
      } else {
        Serial.println("ERROR: getTZ() returned null or empty");
      }
    }
    Serial.println("=== Portal timezone update complete ===");
  } else {
    displayStatus(3);
    #ifdef DEBUG
    Serial.println("Connecting to saved WiFi...");
    #endif
    if (!wifiManager->autoConnect(apSSID.c_str(), apPassword.c_str())) {
      #ifdef DEBUG
      Serial.println("AutoConnect failed - restarting");
      #endif
      delay(3000);
      ESP.restart();
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    #ifdef DEBUG
    Serial.print("WiFi connected - SSID: ");
    Serial.print(WiFi.SSID());
    Serial.print(", IP: ");
    Serial.println(WiFi.localIP());
    #endif
    configureNTP();
    return true;
  }
  return false;
}

void checkWiFiStatus() {
  drd->loop();
  if (WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG
    Serial.println("WiFi disconnected, attempting reconnect...");
    #endif
    WiFi.reconnect();
  }
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void syncRTCWithNTP(ESP32Time& rtc) {
  #ifdef DEBUG
  Serial.println("Syncing RTC with NTP...");
  #endif
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 10000)) {
    rtc.setTimeStruct(timeinfo);
    #ifdef DEBUG
    Serial.print("RTC synced: ");
    Serial.println(rtc.getTime("%Y-%m-%d %H:%M:%S"));
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("Failed to sync RTC with NTP");
    #endif
  }
}
