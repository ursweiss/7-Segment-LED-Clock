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

#ifndef LED_CLOCK_H
#define LED_CLOCK_H

#include <Arduino.h>
#include <FastLED.h>
#include <ESP32Time.h>
#include "config.h"

#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    58

extern CRGB leds[NUM_LEDS];
extern CRGBPalette16 currentPalette;
extern TBlendType currentBlending;
extern CRGB currentColor;
extern uint8_t colorIndex;

void initLEDs();
int mapChar(char character);
void displayCharacter(uint8_t charNum, uint8_t position, bool customize = false, CRGBPalette16 customPalette = RainbowColors_p, uint8_t customBlendIndex = 0);
void displayClockface(String word, bool customize = false, CRGBPalette16 customPalette = RainbowColors_p, uint8_t customBlendIndex = 0);
void displayTime(ESP32Time& rtc);
void displayTemperature();
void displayStatus(uint8_t messageId);
void displayError(uint8_t errorId);
void secondIndicatorOn();
void secondIndicatorOff();
void secondIndicatorDim();
void toggleSecondIndicator();

#endif

#ifndef LED_CLOCK_H
#define LED_CLOCK_H

#include <FastLED.h>
#include "config.h"

// FastLED configuration
#define NUM_LEDS 58
#define LED_TYPE WS2812
#define COLOR_ORDER GRB

// LED array and display parameters
extern CRGB leds[NUM_LEDS];
extern const uint8_t totalCharacters;
extern const uint8_t segmentsPerCharacter;
extern const uint8_t ledsPerSegment;
extern CRGBPalette16 currentPalette;
extern TBlendType currentBlending;
extern CRGB currentColor;
extern CRGB currentDarkColor;
extern uint8_t colorIndex;
extern uint8_t charBlendIndex;
extern uint8_t darkBrightness;
extern bool secondIndicatorState;
extern char displayWord[5];

// 7-segment character mapping (31 characters)
// Array maps character index to 7 segments: {a, b, c, d, e, f, g}
// a=top, b=top-right, c=bottom-right, d=bottom, e=bottom-left, f=top-left, g=middle
extern const uint8_t ledChar[31][7];

// Function declarations
void initLEDs();
int mapChar(char character);
void toggleSecondIndicator();
void secondIndicatorOff();
void displayCharacter(uint8_t charNum, uint8_t position, bool customize = false, CRGBPalette16 customPalette = RainbowColors_p, uint8_t customBlendIndex = 0);
void displayClockface(String word, bool customize = false, CRGBPalette16 customPalette = RainbowColors_p, uint8_t customBlendIndex = 0);
void displayTime();
void displayTemperature();
void displayStatus(uint8_t messageId);
void displayError(uint8_t errorId);

#endif // LED_CLOCK_H
