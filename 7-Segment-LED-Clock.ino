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

#if !(defined(ESP8266) || defined(ESP32))
  #error This code is intended to run on the ESP8266 or ESP32 platform! Please check your Tools->Board setting.
#endif

#include <FastLED.h>
#include <ArduinoJson.h>
#include <CronAlarms.h>
#include "config.h"
#include "LED_Clock.h"
#include "ESPAsync_WiFiManager_DRD.h"


void setup() {
  #ifdef DEBUG
     Serial.begin(115200); while (!Serial); delay(200);
  #endif

  currentPalette = clockColorPalette;
  currentBlending = clockColorBlending;
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(ledBrightness);
  
  initEspAsyncWiFiManager();
  
  Cron.create(clockUpdateSchedule, displayTime,         false);
  
  if (owmTempEnabled == 1) {
    getWeather();
    Cron.create(owmUpdateSchedule, getWeather,          false);
    Cron.create(owmTempSchedule,   displayTemperature,  false);
  }
}


void loop() {
  // Double Reset Detector
  drd->loop();
  // Check WiFi
  check_status();
  
  Cron.delay();
}
