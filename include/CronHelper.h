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

#ifndef CRONHELPER_H
#define CRONHELPER_H

#include <ESP32Time.h>

class CronHelper {
private:
  struct CronSchedule {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t weekday;
  };
  static bool parseCron(const char* cronStr, CronSchedule& schedule);
  static bool matchField(uint8_t cronField, uint8_t currentValue);

public:
  static bool shouldExecute(const char* cronStr, ESP32Time& rtc);
};

#endif
