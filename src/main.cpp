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
#include "config.h"
#include "Logger.h"
#include "LED_Clock.h"
#include "WiFi_Manager.h"
#include "Weather.h"
#include "CronHelper.h"

// Task scheduler
Scheduler taskScheduler;

// RTC for accurate time tracking
ESP32Time rtc;

// Global variables
uint32_t lastTempDisplayTime = 0;
bool tempDisplayActive = false;
uint8_t lastSecond = 255;

// Task callbacks
void updateClockCallback() {
  uint8_t currentSecond = rtc.getSecond();
  if (currentSecond != lastSecond) {
    lastSecond = currentSecond;
    if (owmTempEnabled && CronHelper::shouldExecute(owmTempSchedule, rtc)) {
      if (!tempDisplayActive) {
        tempDisplayActive = true;
        displayTemperature();
      }
    }
    if (owmTempEnabled && CronHelper::shouldExecute(owmUpdateSchedule, rtc)) {
      fetchWeather();
    }
  }
  if (tempDisplayActive && (millis() - lastTempDisplayTime >= (owmTempDisplayTime * 1000))) {
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
  initLEDs();
  if (!initWiFiManager()) {
    LOG_ERROR("WiFi initialization failed");
    displayError(1);
    delay(5000);
    ESP.restart();
  }
  syncRTCWithNTP(rtc);
  setLoggerRTC(&rtc);
  if (owmTempEnabled) {
    fetchWeather();
  }
  taskScheduler.addTask(taskUpdateClock);
  taskUpdateClock.enable();
  LOG_INFO("Setup complete");
}

void loop() {
  static unsigned long lastRTCSync = 0;
  const unsigned long RTC_SYNC_INTERVAL = 3600000;
  if (millis() - lastRTCSync >= RTC_SYNC_INTERVAL) {
    if (isWiFiConnected()) {
      syncRTCWithNTP(rtc);
    }
    lastRTCSync = millis();
  }
  checkWiFiStatus();
  taskScheduler.execute();
}