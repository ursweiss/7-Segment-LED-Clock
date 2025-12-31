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

#include "CronHelper.h"
#include <string.h>
#include <stdlib.h>

// Cache for parsed cron schedules to avoid re-parsing every second
struct CronCache {
  char cronString[64];
  CronHelper::CronSchedule schedule;
  bool valid;
};

static CronCache weatherUpdateCache = {.valid = false};
static CronCache weatherTempCache = {.valid = false};
static CronCache clockUpdateCache = {.valid = false};

bool CronHelper::parseField(const char* fieldStr, CronField& field) {
  // Initialize field
  field.value = 0;
  field.step = 0;
  field.isWildcard = false;

  // Check if field contains step operator '/'
  const char* slashPos = strchr(fieldStr, '/');

  if (slashPos != nullptr) {
    // Step value detected (e.g., "*/15" or "5/15")
    field.step = atoi(slashPos + 1);
    if (field.step == 0) return false; // Invalid step size

    // Check if it's wildcard step (e.g., "*/15")
    if (fieldStr[0] == '*') {
      field.isWildcard = true;
      field.value = 0; // Start from 0
    } else {
      // Offset step (e.g., "5/15")
      field.value = atoi(fieldStr);
    }
  } else if (strcmp(fieldStr, "*") == 0) {
    // Wildcard - matches any value
    field.isWildcard = true;
    field.value = 255;
    field.step = 0;
  } else {
    // Single value (e.g., "5")
    field.value = atoi(fieldStr);
    field.step = 0;
  }

  return true;
}

bool CronHelper::parseCron(const char* cronStr, CronSchedule& schedule) {
  char temp[64];
  strncpy(temp, cronStr, sizeof(temp) - 1);
  temp[sizeof(temp) - 1] = '\0';

  char* token = strtok(temp, " ");
  if (!token || !parseField(token, schedule.second)) return false;

  token = strtok(nullptr, " ");
  if (!token || !parseField(token, schedule.minute)) return false;

  token = strtok(nullptr, " ");
  if (!token || !parseField(token, schedule.hour)) return false;

  token = strtok(nullptr, " ");
  if (!token || !parseField(token, schedule.day)) return false;

  token = strtok(nullptr, " ");
  if (!token || !parseField(token, schedule.month)) return false;

  token = strtok(nullptr, " ");
  if (token) {
    if (!parseField(token, schedule.weekday)) return false;
  } else {
    // Weekday is optional, default to wildcard
    schedule.weekday.isWildcard = true;
    schedule.weekday.value = 255;
    schedule.weekday.step = 0;
  }

  return true;
}

bool CronHelper::matchField(const CronField& cronField, uint8_t currentValue) {
  // Wildcard without step - matches any value
  if (cronField.isWildcard && cronField.step == 0) {
    return true;
  }

  // Step value (e.g., "*/15" or "5/15")
  if (cronField.step > 0) {
    // Check if current value matches the step pattern
    // For "*/15": matches when (currentValue % step) == 0
    // For "5/15": matches when (currentValue - offset) % step == 0 AND currentValue >= offset
    if (cronField.isWildcard) {
      // Wildcard step (e.g., "*/15")
      return (currentValue % cronField.step) == 0;
    } else {
      // Offset step (e.g., "5/15")
      return currentValue >= cronField.value &&
             ((currentValue - cronField.value) % cronField.step) == 0;
    }
  }

  // Single value match
  return cronField.value == currentValue;
}

bool CronHelper::shouldExecute(const char* cronStr, ESP32Time& rtc) {
  // Determine which cache to use based on cron string pattern
  CronCache* cache = nullptr;
  if (strstr(cronStr, "0 5 * * * *") != nullptr) {
    cache = &weatherUpdateCache;
  } else if (strstr(cronStr, "30 * * * * *") != nullptr) {
    cache = &weatherTempCache;
  } else if (strstr(cronStr, "*/5 * * * * *") != nullptr || strstr(cronStr, "* * * * * *") != nullptr) {
    cache = &clockUpdateCache;
  }

  CronSchedule schedule;

  // Use cache if available and valid
  if (cache != nullptr && cache->valid && strcmp(cache->cronString, cronStr) == 0) {
    schedule = cache->schedule;
  } else {
    // Parse and cache
    if (!parseCron(cronStr, schedule)) return false;
    if (cache != nullptr) {
      strncpy(cache->cronString, cronStr, sizeof(cache->cronString) - 1);
      cache->cronString[sizeof(cache->cronString) - 1] = '\0';
      cache->schedule = schedule;
      cache->valid = true;
    }
  }

  struct tm timeinfo = rtc.getTimeStruct();
  return matchField(schedule.second, timeinfo.tm_sec) &&
         matchField(schedule.minute, timeinfo.tm_min) &&
         matchField(schedule.hour, timeinfo.tm_hour) &&
         matchField(schedule.day, timeinfo.tm_mday) &&
         matchField(schedule.month, timeinfo.tm_mon + 1) &&
         matchField(schedule.weekday, timeinfo.tm_wday);
}

void CronHelper::invalidateCache() {
  weatherUpdateCache.valid = false;
  weatherTempCache.valid = false;
  clockUpdateCache.valid = false;
}

bool CronHelper::validateCron(const char* cronStr) {
  CronSchedule schedule;
  return parseCron(cronStr, schedule);
}
