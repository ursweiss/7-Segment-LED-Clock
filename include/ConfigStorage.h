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

#ifndef CONFIG_STORAGE_H
#define CONFIG_STORAGE_H

#include <Arduino.h>

#define TZNAME_MAX_LEN 50
#define TIMEZONE_MAX_LEN 50
#define CONFIG_FILE "/wifi_cred.dat"

struct ClockConfig {
  char TZ_Name[TZNAME_MAX_LEN];
  char TZ[TIMEZONE_MAX_LEN];
  uint16_t checksum;
};

bool loadClockConfig(ClockConfig& config);
bool saveClockConfig(const ClockConfig& config);

#endif
