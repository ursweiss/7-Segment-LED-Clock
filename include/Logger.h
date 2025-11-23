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

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <ESP32Time.h>

enum LogLevel {
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_CRIT,
  LOG_LEVEL_DEBUG
};

void initLogger();
void setLoggerRTC(ESP32Time* rtc);
void logMessage(LogLevel level, String message);
void logMessageF(LogLevel level, const char* format, ...);

// Simple logging macros
#define LOG_INFO(msg) logMessage(LOG_LEVEL_INFO, msg)
#define LOG_WARN(msg) logMessage(LOG_LEVEL_WARN, msg)
#define LOG_ERROR(msg) logMessage(LOG_LEVEL_ERROR, msg)
#define LOG_CRIT(msg) logMessage(LOG_LEVEL_CRIT, msg)

// Printf-style logging macros
#define LOG_INFOF(fmt, ...) logMessageF(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARNF(fmt, ...) logMessageF(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_ERRORF(fmt, ...) logMessageF(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_CRITF(fmt, ...) logMessageF(LOG_LEVEL_CRIT, fmt, ##__VA_ARGS__)

// DEBUG logging - only active when DEBUG is defined
#ifdef DEBUG
#define LOG_DEBUG(msg) logMessage(LOG_LEVEL_DEBUG, msg)
#define LOG_DEBUGF(fmt, ...) logMessageF(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(msg)
#define LOG_DEBUGF(fmt, ...)
#endif

#endif // LOGGER_H
