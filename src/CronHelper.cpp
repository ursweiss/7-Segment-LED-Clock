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

#include "CronHelper.h"
#include <string.h>
#include <stdlib.h>

bool CronHelper::parseCron(const char* cronStr, CronSchedule& schedule) {
  char temp[64];
  strncpy(temp, cronStr, sizeof(temp) - 1);
  temp[sizeof(temp) - 1] = '\0';
  char* token = strtok(temp, " ");
  if (!token) return false;
  schedule.second = (strcmp(token, "*") == 0) ? 255 : atoi(token);
  token = strtok(nullptr, " ");
  if (!token) return false;
  schedule.minute = (strcmp(token, "*") == 0) ? 255 : atoi(token);
  token = strtok(nullptr, " ");
  if (!token) return false;
  schedule.hour = (strcmp(token, "*") == 0) ? 255 : atoi(token);
  token = strtok(nullptr, " ");
  if (!token) return false;
  schedule.day = (strcmp(token, "*") == 0) ? 255 : atoi(token);
  token = strtok(nullptr, " ");
  if (!token) return false;
  schedule.month = (strcmp(token, "*") == 0) ? 255 : atoi(token);
  token = strtok(nullptr, " ");
  schedule.weekday = token ? ((strcmp(token, "*") == 0) ? 255 : atoi(token)) : 255;
  return true;
}

bool CronHelper::matchField(uint8_t cronField, uint8_t currentValue) {
  return cronField == 255 || cronField == currentValue;
}

bool CronHelper::shouldExecute(const char* cronStr, ESP32Time& rtc) {
  CronSchedule schedule;
  if (!parseCron(cronStr, schedule)) return false;
  struct tm timeinfo = rtc.getTimeStruct();
  return matchField(schedule.second, timeinfo.tm_sec) &&
         matchField(schedule.minute, timeinfo.tm_min) &&
         matchField(schedule.hour, timeinfo.tm_hour) &&
         matchField(schedule.day, timeinfo.tm_mday) &&
         matchField(schedule.month, timeinfo.tm_mon + 1) &&
         matchField(schedule.weekday, timeinfo.tm_wday);
}
