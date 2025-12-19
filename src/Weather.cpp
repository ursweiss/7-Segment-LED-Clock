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
#include "WiFi_Manager.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

extern ConfigManager configManager;

int8_t owmTemperature = -128;

void fetchWeather() {
  Config& cfg = configManager.getConfig();

  // Check if weather is enabled
  if (!cfg.weatherTempEnabled) {
    return;
  }

  // Validate coordinates are configured
  if (cfg.locationLatitude.isEmpty() || cfg.locationLongitude.isEmpty()) {
    LOG_WARN("Weather disabled: coordinates not configured");
    return;
  }

  // Check WiFi connection before attempting request
  if (!isWiFiConnected()) {
    LOG_WARN("Weather fetch skipped - WiFi not connected");
    owmTemperature = -128; // Er05 - WiFi disconnected
    return;
  }

  LOG_INFO("Fetching weather from Open-Meteo...");

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(5000);

  String url = "https://api.open-meteo.com/v1/forecast?current=temperature_2m&latitude=" + cfg.locationLatitude + "&longitude=" + cfg.locationLongitude;

  if (!http.begin(client, url)) {
    LOG_ERROR("Failed to initialize HTTPS connection");
    owmTemperature = -127; // Er03
    return;
  }

  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      LOG_DEBUG("Weather API response received");

      DynamicJsonDocument jsonBuffer(2048);
      DeserializationError error = deserializeJson(jsonBuffer, payload);
      if (error) {
        LOG_ERRORF("Weather JSON parsing failed: %s", error.c_str());
        owmTemperature = -127; // Er03
        http.end();
        return;
      }

      // Parse temperature value
      if (!jsonBuffer.containsKey("current") || !jsonBuffer["current"].containsKey("temperature_2m")) {
        LOG_ERROR("Temperature field missing in API response");
        owmTemperature = -127; // Er03
        http.end();
        return;
      }

      float tempValue = jsonBuffer["current"]["temperature_2m"];

      // Parse temperature unit
      if (!jsonBuffer.containsKey("current_units") || !jsonBuffer["current_units"].containsKey("temperature_2m")) {
        LOG_ERROR("Temperature unit missing in API response");
        owmTemperature = -126; // Er04
        http.end();
        return;
      }

      String tempUnit = jsonBuffer["current_units"]["temperature_2m"].as<String>();

      // Determine if conversion needed
      bool isCelsius = tempUnit.indexOf("°C") >= 0 || tempUnit.indexOf("C") >= 0;
      bool isFahrenheit = tempUnit.indexOf("°F") >= 0 || tempUnit.indexOf("F") >= 0;

      if (!isCelsius && !isFahrenheit) {
        LOG_ERRORF("Unrecognized temperature unit: %s", tempUnit.c_str());
        owmTemperature = -126; // Er04
        http.end();
        return;
      }

      // Convert if needed
      bool needMetric = cfg.locationUnits == "metric";
      bool needImperial = cfg.locationUnits == "imperial";

      if ((needMetric && isFahrenheit) || (needImperial && isCelsius)) {
        if (needMetric) {
          // Convert F to C
          tempValue = (tempValue - 32.0) * 5.0 / 9.0;
          LOG_DEBUGF("Converted %s to °C", tempUnit.c_str());
        } else {
          // Convert C to F
          tempValue = (tempValue * 9.0 / 5.0) + 32.0;
          LOG_DEBUGF("Converted %s to °F", tempUnit.c_str());
        }
      }

      // Round to integer
      owmTemperature = (int8_t)round(tempValue);
      LOG_INFOF("Temperature: %d%s", owmTemperature, needMetric ? "°C" : "°F");

    } else {
      LOG_WARNF("Weather API HTTP error: %d", httpCode);
      owmTemperature = -127; // Er03
    }
  } else {
    LOG_ERRORF("Weather API request failed: %s", http.errorToString(httpCode).c_str());
    owmTemperature = -127; // Er03
  }
  http.end();
}
