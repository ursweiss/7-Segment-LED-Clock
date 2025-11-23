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

#include "Logger.h"
#include "config.h"

// Logger state
static ESP32Time* loggerRTC = nullptr;
static bool timestampAvailable = false;

void initLogger() {
  Serial.begin(115200);
  while (!Serial);
  delay(200);
}

void setLoggerRTC(ESP32Time* rtc) {
  loggerRTC = rtc;
  if (loggerRTC != nullptr && loggerRTC->getEpoch() > 1000000000) {
    timestampAvailable = true;
  }
}

void logMessage(LogLevel level, String message) {
  #ifndef DEBUG
  if (level == LOG_LEVEL_DEBUG) {
    return;
  }
  #endif
  char timestamp[32];
  if (timestampAvailable && loggerRTC != nullptr) {
    int year = loggerRTC->getYear();
    int month = loggerRTC->getMonth() + 1;
    int day = loggerRTC->getDay();
    int hour = loggerRTC->getHour(true);
    int minute = loggerRTC->getMinute();
    int second = loggerRTC->getSecond();
    snprintf(timestamp, sizeof(timestamp), "[%04d-%02d-%02d %02d:%02d:%02d]",
             year, month, day, hour, minute, second);
  } else {
    unsigned long uptime = millis();
    snprintf(timestamp, sizeof(timestamp), "[%lu]", uptime);
  }
  const char* levelStr;
  switch (level) {
    case LOG_LEVEL_INFO:  levelStr = "[INFO]"; break;
    case LOG_LEVEL_WARN:  levelStr = "[WARN]"; break;
    case LOG_LEVEL_ERROR: levelStr = "[ERROR]"; break;
    case LOG_LEVEL_CRIT:  levelStr = "[CRIT]"; break;
    case LOG_LEVEL_DEBUG: levelStr = "[DEBUG]"; break;
    default:              levelStr = "[UNKNOWN]"; break;
  }
  Serial.print(timestamp);
  Serial.print(levelStr);
  Serial.print(" ");
  Serial.println(message);
}

void logMessageF(LogLevel level, const char* format, ...) {
  #ifndef DEBUG
  if (level == LOG_LEVEL_DEBUG) {
    return;
  }
  #endif
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  logMessage(level, String(buffer));
}
