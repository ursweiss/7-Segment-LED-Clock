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

#include <Arduino.h>
#include <FastLED.h>
#include <TaskScheduler.h>
#include <ESP32Time.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "BrightnessControl.h"
#include "LED_Clock.h"
#include "WiFi_Manager.h"
#include "WebConfig.h"
#include "Weather.h"
#include "CronHelper.h"

// Task scheduler
Scheduler taskScheduler;

// RTC for accurate time tracking
ESP32Time rtc;

// Web server for configuration (separate from WiFi Manager's portal server)
AsyncWebServer configWebServer(80);

// Global variables
uint32_t lastTempDisplayTime = 0;
bool tempDisplayActive = false;
uint8_t lastSecond = 255;

// Task callbacks
void updateClockCallback() {
  Config& cfg = configManager.getConfig();
  uint8_t currentSecond = rtc.getSecond();
  updateBrightness(rtc);
  if (currentSecond != lastSecond) {
    lastSecond = currentSecond;
    if (cfg.weatherTempEnabled && CronHelper::shouldExecute(cfg.weatherTempSchedule.c_str(), rtc)) {
      if (!tempDisplayActive) {
        tempDisplayActive = true;
        displayTemperature();
      }
    }
    if (cfg.weatherTempEnabled && CronHelper::shouldExecute(cfg.weatherUpdateSchedule.c_str(), rtc)) {
      fetchWeather();
    }
  }
  if (tempDisplayActive && (millis() - lastTempDisplayTime >= (cfg.weatherTempDisplayTime * 1000))) {
    tempDisplayActive = false;
  }
  if (!tempDisplayActive) {
    displayTime(rtc);
  }
}

// Task definitions
Task taskUpdateClock(100, TASK_FOREVER, &updateClockCallback);

void setup() {
  initLogger();
  LOG_INFO("7-Segment LED Clock Starting...");

  // Initialize configuration manager first
  if (!configManager.begin()) {
    LOG_ERROR("Failed to initialize configuration");
    displayError(2);
    delay(5000);
    ESP.restart();
  }

  Config& cfg = configManager.getConfig();

  initLEDs();
  if (!initWiFiManager()) {
    LOG_ERROR("WiFi initialization failed");
    displayError(1);
    delay(5000);
    ESP.restart();
  }
  syncRTCWithNTP(rtc);
  setLoggerRTC(&rtc);

  // Initialize web configuration server
  LOG_INFO("Initializing web server...");
  if (initWebConfig(&configWebServer)) {
    if (startMDNS("ledclock")) {
      configWebServer.begin();
      LOG_INFO("Web server started on port 80");
    }
  }
  initBrightnessControl();
  if (cfg.weatherTempEnabled) {
    fetchWeather();
  }
  taskScheduler.addTask(taskUpdateClock);
  taskUpdateClock.enable();
  LOG_INFO("Setup complete");
}

void loop() {
  static unsigned long lastRTCSync = 0;
  const unsigned long RTC_SYNC_INTERVAL = 3600000;

  // Check for restart request
  if (isRestartRequested()) {
    LOG_WARN("Restarting device...");
    delay(100);
    ESP.restart();
  }

  if (millis() - lastRTCSync >= RTC_SYNC_INTERVAL) {
    if (isWiFiConnected()) {
      syncRTCWithNTP(rtc);
    }
    lastRTCSync = millis();
  }
  checkWiFiStatus();
  taskScheduler.execute();
}
