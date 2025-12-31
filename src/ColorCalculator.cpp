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

#include "ColorCalculator.h"
#include "LED_Clock.h"

// Access cached configuration values from LED_Clock.cpp
extern uint8_t cachedClockColorMode;
extern CRGB cachedClockColorSolid;
extern uint8_t cachedClockColorCharBlend;
extern CRGBPalette16 currentPalette;
extern TBlendType currentBlending;
extern uint8_t colorIndex;

CRGB ColorCalculator::calculateColor(uint8_t brightness, uint8_t colorOffset) {
  if (cachedClockColorMode == 0) {
    return getSolidColor(brightness);
  } else {
    return getPaletteColor(brightness, colorOffset);
  }
}

CRGB ColorCalculator::calculateIndicatorColor(uint8_t brightness) {
  uint8_t colorCorrection = 2 * cachedClockColorCharBlend;
  return calculateColor(brightness, colorCorrection);
}

CRGB ColorCalculator::getSolidColor(uint8_t brightness) {
  if (brightness == 255) {
    return cachedClockColorSolid;
  }

  CRGB adjustedColor = cachedClockColorSolid;
  CHSV tempColorHsv = rgb2hsv_approximate(adjustedColor);
  tempColorHsv.v = brightness;
  hsv2rgb_rainbow(tempColorHsv, adjustedColor);
  return adjustedColor;
}

CRGB ColorCalculator::getPaletteColor(uint8_t brightness, uint8_t offset) {
  extern void updatePaletteFromConfig();
  updatePaletteFromConfig();
  return ColorFromPalette(currentPalette, colorIndex + offset, brightness, currentBlending);
}
