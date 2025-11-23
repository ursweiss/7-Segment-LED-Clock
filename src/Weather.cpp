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
#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

int8_t owmTemperature = -128;

void fetchWeather() {
  if (!owmTempEnabled) {
    return;
  }
  LOG_INFO("Fetching weather from OpenWeatherMap...");
  HTTPClient http;
  String url = "http://" + owmApiServer + "/data/2.5/forecast?q=" + owmLocation + "&appid=" + owmApiKey + "&cnt=1&units=" + owmUnits;
  LOG_DEBUGF("API URL: %s", url.c_str());
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      LOG_DEBUG("API response received");
      StaticJsonDocument<5000> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, payload);
      if (error) {
        LOG_ERRORF("Weather JSON parsing failed: %s", error.c_str());
        http.end();
        return;
      }
      if (jsonBuffer.containsKey("list") && jsonBuffer["list"].size() > 0) {
        float tempRaw = jsonBuffer["list"][0]["main"]["temp"];
        owmTemperature = round(tempRaw);
        LOG_INFOF("Temperature: %d%s", owmTemperature, owmUnits == "metric" ? "°C" : "°F");
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
