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

#include "ConfigStorage.h"
#include "Logger.h"
#include <LittleFS.h>

uint16_t calcChecksum(const uint8_t* data, uint16_t length) {
  uint16_t checksum = 0;
  for (uint16_t i = 0; i < length; i++) {
    checksum += data[i];
  }
  return checksum;
}

bool loadClockConfig(ClockConfig& config) {
  File file = LittleFS.open(CONFIG_FILE, "r");
  if (!file) {
    LOG_DEBUG("Config file not found");
    return false;
  }
  memset(&config, 0, sizeof(ClockConfig));
  size_t bytesRead = file.readBytes((char*)&config, sizeof(ClockConfig));
  file.close();
  if (bytesRead != sizeof(ClockConfig)) {
    LOG_WARN("Config file size mismatch");
    return false;
  }
  uint16_t savedChecksum = config.checksum;
  uint16_t calculatedChecksum = calcChecksum((uint8_t*)&config, sizeof(ClockConfig) - sizeof(config.checksum));
  if (savedChecksum != calculatedChecksum) {
    LOG_ERROR("Config checksum mismatch");
    return false;
  }
  LOG_DEBUG("Config loaded successfully");
  return true;
}

bool saveClockConfig(const ClockConfig& config) {
  ClockConfig tempConfig = config;
  tempConfig.checksum = calcChecksum((uint8_t*)&tempConfig, sizeof(ClockConfig) - sizeof(tempConfig.checksum));
  File file = LittleFS.open(CONFIG_FILE, "w");
  if (!file) {
    LOG_ERROR("Failed to open config file for writing");
    return false;
  }
  size_t bytesWritten = file.write((uint8_t*)&tempConfig, sizeof(ClockConfig));
  file.close();
  if (bytesWritten != sizeof(ClockConfig)) {
    LOG_ERROR("Failed to write complete config");
    return false;
  }
  LOG_DEBUG("Config saved successfully");
  return true;
}
