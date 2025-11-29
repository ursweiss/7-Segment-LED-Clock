#include "BrightnessControl.h"
#include "config.h"
#include "ConfigManager.h"
#include "Logger.h"
#include <FastLED.h>

enum BrightnessState {
  NORMAL,
  FADING_DOWN,
  DIMMED,
  FADING_UP
};

static BrightnessState currentState = NORMAL;
static uint8_t currentMainBrightness = 128;  // Will be set from config
static uint8_t currentColonBrightness = 128;
static unsigned long fadeStartMillis = 0;
static uint8_t lastLoggedSecond = 255;
static bool fadeCompleteLogged = false;

bool parseTime(const char* timeStr, int& hours, int& minutes) {
  return sscanf(timeStr, "%d:%d", &hours, &minutes) == 2;
}

void initBrightnessControl() {
  Config& cfg = configManager.getConfig();
  
  if (!cfg.ledDimEnabled) {
    currentState = NORMAL;
    currentMainBrightness = cfg.ledBrightness;
    currentColonBrightness = cfg.ledBrightness;
    LOG_INFO("Brightness dimming disabled");
    return;
  }
  
  int startHours, startMinutes, endHours, endMinutes;
  bool validStart = parseTime(cfg.ledDimStartTime.c_str(), startHours, startMinutes);
  bool validEnd = parseTime(cfg.ledDimEndTime.c_str(), endHours, endMinutes);
  
  if (!validStart || !validEnd) {
    LOG_ERROR("Invalid dim time format - dimming disabled");
    currentState = NORMAL;
    currentMainBrightness = cfg.ledBrightness;
    currentColonBrightness = cfg.ledBrightness;
    return;
  }
  
  LOG_INFOF("Brightness dimming enabled: %s-%s (fade %ds)", cfg.ledDimStartTime.c_str(), cfg.ledDimEndTime.c_str(), cfg.ledDimFadeDuration);
}

static bool isInTimePeriod(int currentSeconds, int startSeconds, int endSeconds) {
  if (startSeconds < endSeconds) {
    return (currentSeconds >= startSeconds && currentSeconds < endSeconds);
  } else {
    return (currentSeconds >= startSeconds || currentSeconds < endSeconds);
  }
}

static uint8_t interpolateBrightness(uint8_t fromBrightness, uint8_t toBrightness, unsigned long elapsed, unsigned long duration) {
  if (elapsed >= duration) {
    return toBrightness;
  }
  float progress = (float)elapsed / (float)duration;
  int range = toBrightness - fromBrightness;
  return fromBrightness + (uint8_t)(range * progress);
}

void calculateBrightness(int currentSeconds, int dimStartSeconds, int dimEndSeconds, uint8_t currentSecond) {
  Config& cfg = configManager.getConfig();
  
  if (!cfg.ledDimEnabled) {
    currentState = NORMAL;
    currentMainBrightness = cfg.ledBrightness;
    currentColonBrightness = cfg.ledBrightness;
    return;
  }
  
  int fadeDurationSeconds = cfg.ledDimFadeDuration;
  int fadeDownStartSeconds = dimStartSeconds - fadeDurationSeconds;
  int fadeUpEndSeconds = (dimEndSeconds + fadeDurationSeconds) % 86400;
  
  bool inDimPeriod = isInTimePeriod(currentSeconds, dimStartSeconds, dimEndSeconds);
  
  // Handle fade-down period (may wrap around midnight)
  if (fadeDownStartSeconds < 0) {
    fadeDownStartSeconds += 86400;
  }
  bool inFadeDownPeriod = isInTimePeriod(currentSeconds, fadeDownStartSeconds, dimStartSeconds);
  
  // Handle fade-up period
  bool inFadeUpPeriod = isInTimePeriod(currentSeconds, dimEndSeconds, fadeUpEndSeconds);
  
  BrightnessState newState;
  if (inDimPeriod) {
    newState = DIMMED;
  } else if (inFadeDownPeriod) {
    newState = FADING_DOWN;
  } else if (inFadeUpPeriod) {
    newState = FADING_UP;
  } else {
    newState = NORMAL;
  }
  
  if (newState != currentState) {
    currentState = newState;
    fadeStartMillis = millis();
    fadeCompleteLogged = false;
    lastLoggedSecond = 255;
    
    switch (currentState) {
      case DIMMED:
        LOG_INFO("Dimming started");
        break;
      case NORMAL:
        LOG_INFO("Dimming ended");
        break;
      case FADING_DOWN:
        LOG_DEBUG("Fade down started");
        break;
      case FADING_UP:
        LOG_DEBUG("Fade up started");
        break;
    }
  }
  
  switch (currentState) {
    case NORMAL:
      currentMainBrightness = cfg.ledBrightness;
      break;
      
    case DIMMED:
      currentMainBrightness = cfg.ledDimBrightness;
      break;
      
    case FADING_DOWN: {
      unsigned long fadeElapsed = millis() - fadeStartMillis;
      unsigned long fadeDurationMs = (unsigned long)cfg.ledDimFadeDuration * 1000;
      currentMainBrightness = interpolateBrightness(cfg.ledBrightness, cfg.ledDimBrightness, fadeElapsed, fadeDurationMs);
      break;
    }
      
    case FADING_UP: {
      unsigned long fadeElapsed = millis() - fadeStartMillis;
      unsigned long fadeDurationMs = (unsigned long)cfg.ledDimFadeDuration * 1000;
      currentMainBrightness = interpolateBrightness(cfg.ledDimBrightness, cfg.ledBrightness, fadeElapsed, fadeDurationMs);
      break;
    }
  }
  
  if (currentMainBrightness >= cfg.clockSecIndicatorDiff) {
    currentColonBrightness = currentMainBrightness - cfg.clockSecIndicatorDiff;
    if (currentColonBrightness < 10) {
      currentColonBrightness = 10;
    }
  } else {
    currentColonBrightness = currentMainBrightness;
  }
  
  if (currentState == FADING_DOWN || currentState == FADING_UP) {
    unsigned long fadeElapsed = millis() - fadeStartMillis;
    unsigned long fadeDurationMs = (unsigned long)cfg.ledDimFadeDuration * 1000;
    
    if (fadeElapsed >= fadeDurationMs) {
      if (!fadeCompleteLogged) {
        if (currentState == FADING_DOWN) {
          LOG_DEBUG("Fade down complete - brightness reached target");
        } else {
          LOG_DEBUG("Fade up complete - brightness reached target");
        }
        fadeCompleteLogged = true;
      }
    } else {
      if (currentSecond != lastLoggedSecond) {
        if (currentState == FADING_DOWN) {
          LOG_DEBUGF("Fading down: %d/%d (main: %d, colon: %d)", (int)(fadeElapsed/1000), cfg.ledDimFadeDuration, currentMainBrightness, currentColonBrightness);
        } else {
          LOG_DEBUGF("Fading up: %d/%d (main: %d, colon: %d)", (int)(fadeElapsed/1000), cfg.ledDimFadeDuration, currentMainBrightness, currentColonBrightness);
        }
        lastLoggedSecond = currentSecond;
      }
    }
  }
}

void updateBrightness(ESP32Time& rtc) {
  Config& cfg = configManager.getConfig();
  
  if (!cfg.ledDimEnabled) {
    currentMainBrightness = cfg.ledBrightness;
    currentColonBrightness = cfg.ledBrightness;
    FastLED.setBrightness(cfg.ledBrightness);
    return;
  }
  
  int startHours, startMinutes, endHours, endMinutes;
  bool validStart = parseTime(cfg.ledDimStartTime.c_str(), startHours, startMinutes);
  bool validEnd = parseTime(cfg.ledDimEndTime.c_str(), endHours, endMinutes);
  
  if (!validStart || !validEnd) {
    currentMainBrightness = cfg.ledBrightness;
    currentColonBrightness = cfg.ledBrightness;
    FastLED.setBrightness(cfg.ledBrightness);
    return;
  }
  
  int dimStart = startHours * 3600 + startMinutes * 60;
  int dimEnd = endHours * 3600 + endMinutes * 60;
  int currentSeconds = rtc.getHour(true) * 3600 + rtc.getMinute() * 60 + rtc.getSecond();
  uint8_t currentSecond = rtc.getSecond();
  
  calculateBrightness(currentSeconds, dimStart, dimEnd, currentSecond);
  
  FastLED.setBrightness(currentMainBrightness);
}

uint8_t getCurrentMainBrightness() {
  return currentMainBrightness;
}

uint8_t getCurrentColonBrightness() {
  return currentColonBrightness;
}
