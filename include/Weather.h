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

#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include "config.h"

/**
 * Weather status codes
 * Replaces magic integer values with type-safe enum
 */
enum class WeatherStatus : int8_t {
  Valid = 0,              // Temperature data is valid
  NotYetFetched = -128,   // No data fetched yet (initial state)
  APIFailed = -127,       // HTTP request or JSON parsing failed (Er03)
  InvalidUnit = -126,     // Temperature unit not recognized (Er04)
  WiFiDisconnected = -125 // WiFi not connected (Er05)
};

// External variable for temperature
extern int8_t owmTemperature;

// Helper functions for weather status
inline bool isWeatherError(int8_t temp) {
  // Check if temp exactly matches one of the error enum values
  return temp == static_cast<int8_t>(WeatherStatus::APIFailed) ||
         temp == static_cast<int8_t>(WeatherStatus::InvalidUnit) ||
         temp == static_cast<int8_t>(WeatherStatus::WiFiDisconnected);
}

inline const char* getWeatherErrorCode(int8_t temp) {
  switch (static_cast<WeatherStatus>(temp)) {
    case WeatherStatus::APIFailed: return "Er03";
    case WeatherStatus::InvalidUnit: return "Er04";
    case WeatherStatus::WiFiDisconnected: return "Er05";
    default: return "Err";
  }
}

// Function declarations
void fetchWeather();

#endif // WEATHER_H
