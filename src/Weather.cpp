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
#include <HTTPClient.h>
#include <ArduinoJson.h>

int8_t owmTemperature = -128;

void fetchWeather() {
  if (!owmTempEnabled) {
    return;
  }
  #ifdef DEBUG
  Serial.println("Fetching weather from OpenWeatherMap...");
  #endif
  HTTPClient http;
  String url = "http://" + owmApiServer + "/data/2.5/forecast?q=" + owmLocation + "&appid=" + owmApiKey + "&cnt=1&units=" + owmUnits;
  #ifdef DEBUG
  Serial.print("URL: ");
  Serial.println(url);
  #endif
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      #ifdef DEBUG
      Serial.println("Response received");
      #endif
      StaticJsonDocument<5000> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, payload);
      if (error) {
        #ifdef DEBUG
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        #endif
        http.end();
        return;
      }
      if (jsonBuffer.containsKey("list") && jsonBuffer["list"].size() > 0) {
        float tempRaw = jsonBuffer["list"][0]["main"]["temp"];
        owmTemperature = round(tempRaw);
        #ifdef DEBUG
        Serial.print("Temperature: ");
        Serial.print(owmTemperature);
        Serial.println(owmUnits == "metric" ? "°C" : "°F");
        #endif
      } else {
        #ifdef DEBUG
        Serial.println("Invalid JSON structure");
        #endif
      }
    } else {
      #ifdef DEBUG
      Serial.print("HTTP error code: ");
      Serial.println(httpCode);
      #endif
    }
  } else {
    #ifdef DEBUG
    Serial.print("HTTP request failed: ");
    Serial.println(http.errorToString(httpCode));
    #endif
  }
  http.end();
}
