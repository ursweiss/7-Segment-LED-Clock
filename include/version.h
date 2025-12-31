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

#ifndef VERSION_H
#define VERSION_H

#include <Arduino.h>

// Build timestamp format: YYYY-MM-DD_HHmm
// Generated automatically from compiler __DATE__ and __TIME__ macros
// Example: "2025-11-29_1423"

inline const char* getBuildVersion() {
  static char version[20] = {0};

  if (version[0] == 0) {
    // __DATE__ format: "Nov 29 2025"
    // __TIME__ format: "14:23:45"

    const char* date = __DATE__;
    const char* time = __TIME__;

    // Parse month
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    int month = 1;
    for (int i = 0; i < 12; i++) {
      if (date[0] == months[i][0] && date[1] == months[i][1] && date[2] == months[i][2]) {
        month = i + 1;
        break;
      }
    }

    // Parse day (skip leading space if present)
    int day = (date[4] == ' ' ? 0 : (date[4] - '0') * 10) + (date[5] - '0');

    // Parse year
    int year = (date[7] - '0') * 1000 + (date[8] - '0') * 100 +
               (date[9] - '0') * 10 + (date[10] - '0');

    // Parse hour and minute
    int hour = (time[0] - '0') * 10 + (time[1] - '0');
    int min = (time[3] - '0') * 10 + (time[4] - '0');

    // Format: YYYY-MM-DD_HHmm
    snprintf(version, sizeof(version), "%04d-%02d-%02d_%02d%02d",
             year, month, day, hour, min);
  }

  return version;
}

#endif // VERSION_H
