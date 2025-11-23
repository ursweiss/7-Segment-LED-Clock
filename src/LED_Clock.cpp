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

#include "LED_Clock.h"
#include "Logger.h"
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

// 7-segment character mapping
// 0-9: Digits, 10-16: Hex A-F, 17-18: H/h, 19: L, 20: n, 21-22: O/o
// 23: P, 24: r, 25: S, 26-27: U/u, 28: degree, 29: minus, 30: off
const uint8_t ledChar[31][7] = {
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

void initLEDs() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(ledBrightness);
  currentPalette = clockColorPalette;
  currentBlending = clockColorBlending;
  charBlendIndex = colorIndex;
}

int mapChar(char character) {
  switch (character) {
    case 'A':
    case 'a':
      return 10; break;
    case 'B':
    case 'b':
      return 11; break;
    case 'C':
      return 12; break;
    case 'c':
      return 13; break;
    case 'D':
    case 'd':
      return 14; break;
    case 'E':
    case 'e':
      return 15; break;
    case 'F':
    case 'f':
      return 16; break;
    case 'H':
      return 17; break;
    case 'h':
      return 18; break;
    case 'I':
    case 'i':
      return 1; break;
    case 'L':
      return 19; break;
    case 'l':
      return 1; break;
    case 'N':
    case 'n':
      return 20; break;
    case 'O':
      return 21; break;
    case 'o':
      return 22; break;
    case 'P':
    case 'p':
      return 23; break;
    case 'R':
    case 'r':
      return 24; break;
    case 'S':
    case 's':
      return 25; break;
    case 'U':
      return 26; break;
    case 'u':
      return 27; break;
    case 'z':
      return 28; break;
    case '-':
      return 29; break;
    default:
      return 30; break;
  }
}

void toggleSecondIndicator() {
  uint8_t colorCorrection = 0;
  CRGB tmpColor;
  CRGB tmpDarkColor;
  darkBrightness = ledBrightness - clockSecIndicatorDiff;
  if (darkBrightness > ledBrightness) {
    darkBrightness = 0;
  }
  if (clockColorMode == 0) {
    tmpColor = clockColorSolid;
    tmpDarkColor = clockColorSolid;
    CHSV tempColorHsv = rgb2hsv_approximate(tmpColor);
    tempColorHsv.v = darkBrightness;
    hsv2rgb_rainbow(tempColorHsv, tmpDarkColor);
  } else if (clockColorMode == 1) {
    colorCorrection = 2 * clockColorCharBlend;
    tmpColor = ColorFromPalette(currentPalette, (colorIndex + colorCorrection), ledBrightness, currentBlending);
    tmpDarkColor = ColorFromPalette(currentPalette, (colorIndex + colorCorrection), darkBrightness, currentBlending);
  }
  if (secondIndicatorState) {
    fill_solid(&(leds[NUM_LEDS-2]), 2, tmpDarkColor);
  } else {
    fill_solid(&(leds[NUM_LEDS-2]), 2, tmpColor);
  }
  secondIndicatorState = !secondIndicatorState;
}

void secondIndicatorOn() {
  uint8_t colorCorrection = 0;
  CRGB tmpColor;
  if (clockColorMode == 0) {
    tmpColor = clockColorSolid;
  } else if (clockColorMode == 1) {
    colorCorrection = 2 * clockColorCharBlend;
    tmpColor = ColorFromPalette(currentPalette, (colorIndex + colorCorrection), ledBrightness, currentBlending);
  }
  fill_solid(&(leds[NUM_LEDS-2]), 2, tmpColor);
}

void secondIndicatorOff() {
  fill_solid(&(leds[NUM_LEDS-2]), 2, CRGB::Black);
}

void secondIndicatorDim() {
  uint8_t colorCorrection = 0;
  CRGB tmpDarkColor;
  darkBrightness = ledBrightness - clockSecIndicatorDiff;
  if (darkBrightness > ledBrightness) {
    darkBrightness = 0;
  }
  if (clockColorMode == 0) {
    tmpDarkColor = clockColorSolid;
    CHSV tempColorHsv = rgb2hsv_approximate(tmpDarkColor);
    tempColorHsv.v = darkBrightness;
    hsv2rgb_rainbow(tempColorHsv, tmpDarkColor);
  } else if (clockColorMode == 1) {
    colorCorrection = 2 * clockColorCharBlend;
    tmpDarkColor = ColorFromPalette(currentPalette, (colorIndex + colorCorrection), darkBrightness, currentBlending);
  }
  fill_solid(&(leds[NUM_LEDS-2]), 2, tmpDarkColor);
}

void displayCharacter(uint8_t charNum, uint8_t position, bool customize, CRGBPalette16 customPalette, uint8_t customBlendIndex) {
  if (charNum > 30) {
    return;
  }
  uint8_t offset = position * segmentsPerCharacter * ledsPerSegment;
  for (int i = 0; i < segmentsPerCharacter; i++) {
    if (customize) {
      currentColor = ColorFromPalette(customPalette, customBlendIndex, ledBrightness, currentBlending);
    } else {
      if (clockColorMode == 0) {
        currentColor = clockColorSolid;
      } else if (clockColorMode == 1) {
        currentColor = ColorFromPalette(currentPalette, charBlendIndex, ledBrightness, currentBlending);
      }
    }
    if (ledChar[charNum][i]) {
      fill_solid(&(leds[i*2+offset]), 2, currentColor);
    } else {
      fill_solid(&(leds[i*2+offset]), 2, CRGB::Black);
    }
  }
}

void displayClockface(String word, bool customize, CRGBPalette16 customPalette, uint8_t customBlendIndex) {
  uint8_t charNum;
  char singleChar;
  uint8_t leadingBlanks = totalCharacters - word.length();
  for (int i = 0; i < totalCharacters; i++) {
    if (i < leadingBlanks) {
      charNum = 30;
    } else {
      singleChar = word.charAt(i - leadingBlanks);
      charNum = isDigit(singleChar) ? (uint8_t)singleChar - 48 : mapChar(singleChar);
      charBlendIndex += clockColorCharBlend;
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
  if (currentSecond != lastDisplaySecond) {
    lastDisplaySecond = currentSecond;
    secondIndicatorState = !secondIndicatorState;
    if (clockColorMode == 1) {
      colorIndex++;
    }
  }
  currentColor = ColorFromPalette(currentPalette, colorIndex, ledBrightness, currentBlending);
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
  if (clockSecIndicatorDiff > 0) {
    if (secondIndicatorState) {
      secondIndicatorOn();
    } else {
      secondIndicatorDim();
    }
  }
  FastLED.show();
}

void displayTemperature() {
  if (owmTemperature == -128) {
    return;
  }
  secondIndicatorOff();
  uint8_t customBlendIndex;
  int8_t owmTemperature2 = owmTemperature;
  bool negative = owmTemperature < 0;
  if (owmTemperature < owmTempMin) {
    customBlendIndex = 160;
  } else if (owmTemperature > owmTempMax) {
    customBlendIndex = 0;
  } else if (negative) {
    customBlendIndex = map(owmTemperature, owmTempMin, (owmUnits == "metric" ? -1 : 33), 160, 127);
  } else {
    customBlendIndex = map(owmTemperature, (owmUnits == "metric" ? 0 : 32), owmTempMax, 126, 0);
  }
  if (negative) {
    owmTemperature2 = owmTemperature * -1;
  }
  int tempNibble10 = owmTemperature2 / 10;
  int tempNibble = owmTemperature2 % 10;
  if (negative && tempNibble10 < 1) {
    sprintf(displayWord, "%c%d%c%c", '-', tempNibble, 'z', (owmUnits == "metric" ? 'C' : 'F'));
  } else {
    sprintf(displayWord, "%d%d%c%c", tempNibble10, tempNibble, 'z', (owmUnits == "metric" ? 'C' : 'F'));
  }
  displayClockface(displayWord, true, RainbowColors_p, customBlendIndex);
  FastLED.show();
  lastTempDisplayTime = millis();
}

void displayStatus(uint8_t messageId) {
  String serialMessage;
  String displayMessage;
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
  }
  #ifdef DEBUG
  Serial.println(serialMessage);
  #endif
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
  #ifdef DEBUG
  Serial.println(serialMessage);
  #endif
  sprintf(displayWord, "Er%d%d", tempNibble10, tempNibble);
  displayClockface(displayWord);
  FastLED.show();
  delay(3000);
}
