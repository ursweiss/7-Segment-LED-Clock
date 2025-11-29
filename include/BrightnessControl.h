#ifndef BRIGHTNESS_CONTROL_H
#define BRIGHTNESS_CONTROL_H

#include <ESP32Time.h>

// Public API
bool parseTime(const char* timeStr, int& hours, int& minutes);
void initBrightnessControl();
void updateBrightness(ESP32Time& rtc);
uint8_t getCurrentMainBrightness();
uint8_t getCurrentColonBrightness();

#endif // BRIGHTNESS_CONTROL_H
