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

#include "Weather.h"
#include "Logger.h"
#include "ConfigManager.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

extern ConfigManager configManager;

int8_t owmTemperature = -128;

void fetchWeather() {
  Config& cfg = configManager.getConfig();
  if (!cfg.owmTempEnabled) {
    return;
  }
  LOG_INFO("Fetching weather from OpenWeatherMap...");
  
  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate validation for simplicity
  
  HTTPClient http;
  String url = "https://" + cfg.owmApiServer + "/data/2.5/forecast?q=" + cfg.owmLocation + "&appid=" + cfg.owmApiKey + "&cnt=1&units=" + cfg.owmUnits;
  
  if (!http.begin(client, url)) {
    LOG_ERROR("Failed to initialize HTTPS connection");
    return;
  }
  
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      LOG_DEBUG("API response received");
      DynamicJsonDocument jsonBuffer(5000);
      DeserializationError error = deserializeJson(jsonBuffer, payload);
      if (error) {
        LOG_ERRORF("Weather JSON parsing failed: %s", error.c_str());
        http.end();
        return;
      }
      if (jsonBuffer.containsKey("list") && jsonBuffer["list"].size() > 0) {
        JsonObject main = jsonBuffer["list"][0]["main"];
        if (main.containsKey("temp")) {
          float tempRaw = main["temp"];
          owmTemperature = round(tempRaw);
          LOG_INFOF("Temperature: %d%s", owmTemperature, cfg.owmUnits == "metric" ? "°C" : "°F");
        } else {
          LOG_ERROR("Temperature field missing in API response");
        }
      } else {
        LOG_ERROR("Weather API returned invalid JSON structure");
      }
    } else {
      LOG_WARNF("Weather API HTTP error: %d", httpCode);
    }
  } else {
    LOG_ERRORF("Weather API request failed: %s", http.errorToString(httpCode).c_str());
  }
  http.end();
}
