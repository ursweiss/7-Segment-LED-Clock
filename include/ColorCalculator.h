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

#ifndef COLOR_CALCULATOR_H
#define COLOR_CALCULATOR_H

#include <FastLED.h>

/**
 * Utility class for calculating LED colors based on mode (solid/palette)
 * Eliminates duplication across second indicator functions
 */
class ColorCalculator {
public:
  /**
   * Calculate color for display based on current mode and brightness
   * @param brightness Target brightness level (0-255)
   * @param colorOffset Optional offset for palette color calculation
   * @return Calculated CRGB color
   */
  static CRGB calculateColor(uint8_t brightness, uint8_t colorOffset = 0);

  /**
   * Calculate color for second indicator (uses 2x char blend offset)
   * @param brightness Target brightness level (0-255)
   * @return Calculated CRGB color
   */
  static CRGB calculateIndicatorColor(uint8_t brightness);

private:
  static CRGB getSolidColor(uint8_t brightness);
  static CRGB getPaletteColor(uint8_t brightness, uint8_t offset);
};

#endif // COLOR_CALCULATOR_H
