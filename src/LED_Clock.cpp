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

#include "LED_Clock.h"
#include "Logger.h"
#include "BrightnessControl.h"
#include "ConfigManager.h"
#include "ColorCalculator.h"
#include "Weather.h"
#include <ESP32Time.h>

// Global variables
CRGB leds[NUM_LEDS];
const uint8_t totalCharacters = 4;
const uint8_t segmentsPerCharacter = 7;
const uint8_t ledsPerSegment = 2;
CRGBPalette16 currentPalette;
TBlendType currentBlending;
CRGB currentColor;
CRGB currentDarkColor;
uint8_t colorIndex = 0;
uint8_t charBlendIndex;
uint8_t darkBrightness = 0;
bool secondIndicatorState = true;
char displayWord[5];
extern int8_t owmTemperature;
extern uint32_t lastTempDisplayTime;

// Cached configuration values to avoid repeated getConfig() calls
// Non-static to allow access from ColorCalculator
bool paletteNeedsUpdate = true;
uint8_t cachedClockColorMode = 1;
CRGB cachedClockColorSolid = CRGB::Green;
uint8_t cachedClockColorCharBlend = 5;
uint8_t cachedClockSecIndicatorDiff = 32;

// 7-segment character mapping
// 0-9: Digits, 10-16: Hex A-F, 17-18: H/h, 19: L, 20: n, 21-22: O/o
// 23: P, 24: r, 25: S, 26-27: U/u, 28: degree, 29: minus, 30: off
const uint8_t PROGMEM ledChar[31][7] = {
  {0,1,1,1,1,1,1}, // 0
  {0,1,0,0,0,0,1}, // 1, l, I
  {1,1,1,0,1,1,0}, // 2
  {1,1,1,0,0,1,1}, // 3
  {1,1,0,1,0,0,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {0,1,1,0,0,0,1}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}, // 9
  {1,1,1,1,1,0,1}, // A (10)
  {1,0,0,1,1,1,1}, // b
  {0,0,1,1,1,1,0}, // C
  {1,0,0,0,1,1,0}, // c
  {1,1,0,0,1,1,1}, // d
  {1,0,1,1,1,1,0}, // E (15)
  {1,0,1,1,1,0,0}, // F
  {1,1,0,1,1,0,1}, // H
  {1,0,0,1,1,0,1}, // h
  {0,0,0,1,1,1,0}, // L
  {1,0,0,0,1,0,1}, // n (20)
  {0,1,1,1,1,1,1}, // O
  {1,0,0,0,1,1,1}, // o
  {1,1,1,1,1,0,0}, // P
  {1,0,0,0,1,0,0}, // r
  {1,0,1,1,0,1,1}, // S (25)
  {0,1,0,1,1,1,1}, // U
  {0,0,0,0,1,1,1}, // u
  {1,1,1,1,0,0,0}, // degree (°)
  {1,0,0,0,0,0,0}, // minus (-)
  {0,0,0,0,0,0,0}  // off (30)
};

void updatePaletteFromConfig() {
  if (!paletteNeedsUpdate) return;

  Config& cfg = configManager.getConfig();
  currentPalette = ConfigManager::getPaletteByIndex(cfg.clockColorPaletteIndex);
  currentBlending = (cfg.clockColorBlending == 1) ? LINEARBLEND : NOBLEND;

  // Cache frequently accessed config values
  cachedClockColorMode = cfg.clockColorMode;
  cachedClockColorSolid = cfg.clockColorSolid;
  cachedClockColorCharBlend = cfg.clockColorCharBlend;
  cachedClockSecIndicatorDiff = cfg.clockSecIndicatorDiff;

  paletteNeedsUpdate = false;
}

void markPaletteForUpdate() {
  paletteNeedsUpdate = true;
}

void initLEDs() {
  Config& cfg = configManager.getConfig();
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(cfg.ledBrightness);
  updatePaletteFromConfig();
  charBlendIndex = colorIndex;
}

// Lookup table for character mapping (O(1) instead of O(n) switch)
static int8_t charMap[128] = {0};
static bool charMapInitialized = false;

void initCharMap() {
  if (charMapInitialized) return;
  charMap['A'] = 10; charMap['a'] = 10;
  charMap['B'] = 11; charMap['b'] = 11;
  charMap['C'] = 12; charMap['c'] = 13;
  charMap['D'] = 14; charMap['d'] = 14;
  charMap['E'] = 15; charMap['e'] = 15;
  charMap['F'] = 16; charMap['f'] = 16;
  charMap['H'] = 17; charMap['h'] = 18;
  charMap['I'] = 1;  charMap['i'] = 1;
  charMap['L'] = 19; charMap['l'] = 1;
  charMap['N'] = 20; charMap['n'] = 20;
  charMap['O'] = 21; charMap['o'] = 22;
  charMap['P'] = 23; charMap['p'] = 23;
  charMap['R'] = 24; charMap['r'] = 24;
  charMap['S'] = 25; charMap['s'] = 25;
  charMap['U'] = 26; charMap['u'] = 27;
  charMap['z'] = 28;
  charMap['-'] = 29;
  charMapInitialized = true;
}

int mapChar(char character) {
  initCharMap();
  if (character >= 0 && character < 128) {
    int8_t mapped = charMap[(uint8_t)character];
    return (mapped != 0) ? mapped : 30;
  }
  return 30;
}

void toggleSecondIndicator() {
  Config& cfg = configManager.getConfig();
  CRGB brightColor = ColorCalculator::calculateIndicatorColor(cfg.ledBrightness);

  uint8_t dimBrightness = cfg.ledBrightness - cachedClockSecIndicatorDiff;
  if (dimBrightness > cfg.ledBrightness) {
    dimBrightness = 0;
  }
  CRGB dimColor = ColorCalculator::calculateIndicatorColor(dimBrightness);

  fill_solid(&(leds[NUM_LEDS-2]), 2, secondIndicatorState ? dimColor : brightColor);
  secondIndicatorState = !secondIndicatorState;
}

void secondIndicatorOn() {
  uint8_t currentBrightness = getCurrentMainBrightness();
  CRGB color = ColorCalculator::calculateIndicatorColor(currentBrightness);
  fill_solid(&(leds[NUM_LEDS-2]), 2, color);
}

void secondIndicatorOff() {
  fill_solid(&(leds[NUM_LEDS-2]), 2, CRGB::Black);
}

void secondIndicatorDim() {
  uint8_t dimBrightness = getCurrentColonBrightness();
  CRGB color = ColorCalculator::calculateIndicatorColor(dimBrightness);
  fill_solid(&(leds[NUM_LEDS-2]), 2, color);
}

void displayCharacter(uint8_t charNum, uint8_t position, bool customize, CRGBPalette16 customPalette, uint8_t customBlendIndex) {
  // Bounds check: character must be valid and position must not overflow LED array
  if (charNum > 30 || position >= totalCharacters) {
    #ifdef DEBUG
    if (charNum > 30) {
      LOG_WARNF("Invalid character number: %d", charNum);
    }
    if (position >= totalCharacters) {
      LOG_WARNF("Invalid position: %d (max: %d)", position, totalCharacters - 1);
    }
    #endif
    return;
  }

  // Calculate color ONCE before loop instead of 7 times
  CRGB segmentColor;
  if (customize) {
    Config& cfg = configManager.getConfig();
    segmentColor = ColorFromPalette(customPalette, customBlendIndex, cfg.ledBrightness, currentBlending);
  } else {
    if (cachedClockColorMode == 0) {
      segmentColor = cachedClockColorSolid;
    } else if (cachedClockColorMode == 1) {
      updatePaletteFromConfig();
      Config& cfg = configManager.getConfig();
      segmentColor = ColorFromPalette(currentPalette, charBlendIndex, cfg.ledBrightness, currentBlending);
    }
  }

  uint8_t offset = position * segmentsPerCharacter * ledsPerSegment;
  for (int i = 0; i < segmentsPerCharacter; i++) {
    if (pgm_read_byte(&ledChar[charNum][i])) {
      fill_solid(&(leds[i*2+offset]), 2, segmentColor);
    } else {
      fill_solid(&(leds[i*2+offset]), 2, CRGB::Black);
    }
  }
}

void displayClockface(const char* word, bool customize, CRGBPalette16 customPalette, uint8_t customBlendIndex) {
  uint8_t charNum;
  char singleChar;
  uint8_t wordLen = strlen(word);
  uint8_t leadingBlanks = totalCharacters - wordLen;
  for (int i = 0; i < totalCharacters; i++) {
    if (i < leadingBlanks) {
      charNum = 30;
    } else {
      singleChar = word[i - leadingBlanks];
      charNum = isDigit(singleChar) ? (uint8_t)singleChar - 48 : mapChar(singleChar);
      charBlendIndex += cachedClockColorCharBlend;
    }
    displayCharacter(charNum, i, customize, customPalette, customBlendIndex);
  }
  charBlendIndex = colorIndex;
}

void displayTime(ESP32Time& rtc) {
  static uint8_t lastDisplaySecond = 255;
  uint8_t currentSecond = rtc.getSecond();
  int currentHour = rtc.getHour(true);
  int currentMinute = rtc.getMinute();

  // Bounds check: validate time values are within expected ranges
  if (currentHour < 0 || currentHour > 23 || currentMinute < 0 || currentMinute > 59) {
    #ifdef DEBUG
    LOG_WARNF("Invalid time values: %02d:%02d", currentHour, currentMinute);
    #endif
    return;
  }
  if (currentSecond != lastDisplaySecond) {
    lastDisplaySecond = currentSecond;
    secondIndicatorState = !secondIndicatorState;
    if (cachedClockColorMode == 1) {
      colorIndex++;
    }
  }
  uint8_t currentBrightness = getCurrentMainBrightness();
  currentColor = ColorFromPalette(currentPalette, colorIndex, currentBrightness, currentBlending);
  int hourNibble10 = currentHour / 10;
  int hourNibble = currentHour % 10;
  int minNibble10 = currentMinute / 10;
  int minNibble = currentMinute % 10;
  if (hourNibble10 == 0) {
    sprintf(displayWord, "%d%d%d", hourNibble, minNibble10, minNibble);
  } else {
    sprintf(displayWord, "%d%d%d%d", hourNibble10, hourNibble, minNibble10, minNibble);
  }
  displayClockface(displayWord);
  if (cachedClockSecIndicatorDiff > 0) {
    if (secondIndicatorState) {
      secondIndicatorOn();
    } else {
      secondIndicatorDim();
    }
  }
  FastLED.show();
}

// Helper functions for temperature display
static bool formatTemperatureDisplay(int8_t temp, char* output, size_t outputSize) {
  if (isWeatherError(temp)) {
    strncpy(output, getWeatherErrorCode(temp), outputSize - 1);
    output[outputSize - 1] = '\0';
    return true;
  }
  if (temp == static_cast<int8_t>(WeatherStatus::NotYetFetched)) {
    return false;
  }
  return true;
}

static uint8_t calculateTempColorIndex(int8_t temp) {
  Config& cfg = configManager.getConfig();
  bool negative = temp < 0;

  if (temp < cfg.weatherTempMin) {
    return 160;
  }
  if (temp > cfg.weatherTempMax) {
    return 0;
  }

  bool isMetric = (cfg.locationUnits == "metric");
  if (negative) {
    return map(temp, cfg.weatherTempMin, (isMetric ? -1 : 33), 160, 127);
  } else {
    return map(temp, (isMetric ? 0 : 32), cfg.weatherTempMax, 126, 0);
  }
}

static void buildTempString(int8_t temp, bool isMetric, char* output) {
  bool negative = temp < 0;
  int8_t absTemp = negative ? -temp : temp;
  int tempNibble10 = absTemp / 10;
  int tempNibble = absTemp % 10;
  char unit = isMetric ? 'C' : 'F';

  if (negative && tempNibble10 < 1) {
    sprintf(output, "%c%d%c%c", '-', tempNibble, 'z', unit);
  } else if (negative) {
    sprintf(output, "%c%d%c%c", '-', absTemp, 'z', unit);
  } else if (tempNibble10 < 1) {
    sprintf(output, "%c%d%c%c", ' ', tempNibble, 'z', unit);
  } else {
    sprintf(output, "%d%d%c%c", tempNibble10, tempNibble, 'z', unit);
  }
}

void displayTemperature() {
  Config& cfg = configManager.getConfig();

  if (!formatTemperatureDisplay(owmTemperature, displayWord, sizeof(displayWord))) {
    return;
  }

  if (isWeatherError(owmTemperature)) {
    displayClockface(displayWord);
    FastLED.show();
    lastTempDisplayTime = millis();
    return;
  }

  secondIndicatorOff();
  uint8_t customBlendIndex = calculateTempColorIndex(owmTemperature);
  bool isMetric = (cfg.locationUnits == "metric");
  buildTempString(owmTemperature, isMetric, displayWord);
  displayClockface(displayWord, true, RainbowColors_p, customBlendIndex);
  FastLED.show();
  lastTempDisplayTime = millis();
}

void displayStatus(uint8_t messageId) {
  const char* serialMessage;
  const char* displayMessage;
  switch (messageId) {
    case 1:
      serialMessage = "Initialize";
      displayMessage = "Load";
      break;
    case 2:
      serialMessage = "Started config portal";
      displayMessage = "Conf";
      break;
    case 3:
      serialMessage = "Connecting to WiFi";
      displayMessage = "Conn";
      break;
    default:
      serialMessage = "Unknown status";
      displayMessage = "----";
      break;
  }
  LOG_INFO(serialMessage);
  displayClockface(displayMessage);
  FastLED.show();
}

void displayError(uint8_t errorId) {
  String serialMessage;
  switch (errorId) {
    case 1:
      serialMessage = "Error 01";
      break;
  }
  int tempNibble10 = errorId / 10;
  int tempNibble = errorId % 10;
  LOG_ERROR(serialMessage.c_str());
  sprintf(displayWord, "Er%d%d", tempNibble10, tempNibble);
  displayClockface(displayWord);
  FastLED.show();
  delay(3000);
}
