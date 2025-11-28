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

int parseTimeToSeconds(const char* timeStr) {
  int hours = 0, minutes = 0;
  if (sscanf(timeStr, "%d:%d", &hours, &minutes) == 2) {
    return hours * 3600 + minutes * 60;
  }
  return -1;
}

int parseTimeToMinutes(const char* timeStr) {
  int hours = 0, minutes = 0;
  if (sscanf(timeStr, "%d:%d", &hours, &minutes) == 2) {
    return hours * 60 + minutes;
  }
  return -1;
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
  
  int dimStart = parseTimeToMinutes(cfg.ledDimStartTime.c_str());
  int dimEnd = parseTimeToMinutes(cfg.ledDimEndTime.c_str());
  
  if (dimStart == -1 || dimEnd == -1) {
    LOG_ERROR("Invalid dim time format - dimming disabled");
    currentState = NORMAL;
    currentMainBrightness = cfg.ledBrightness;
    currentColonBrightness = cfg.ledBrightness;
    return;
  }
  
  LOG_INFOF("Brightness dimming enabled: %s-%s (fade %ds)", cfg.ledDimStartTime.c_str(), cfg.ledDimEndTime.c_str(), cfg.ledDimFadeDuration);
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
  
  bool inDimPeriod = false;
  if (dimStartSeconds < dimEndSeconds) {
    inDimPeriod = (currentSeconds >= dimStartSeconds && currentSeconds < dimEndSeconds);
  } else {
    inDimPeriod = (currentSeconds >= dimStartSeconds || currentSeconds < dimEndSeconds);
  }
  
  bool inFadeDownPeriod = false;
  if (fadeDownStartSeconds < 0) {
    fadeDownStartSeconds += 86400;
    inFadeDownPeriod = (currentSeconds >= fadeDownStartSeconds || currentSeconds < dimStartSeconds);
  } else {
    if (fadeDownStartSeconds < dimStartSeconds) {
      inFadeDownPeriod = (currentSeconds >= fadeDownStartSeconds && currentSeconds < dimStartSeconds);
    } else {
      inFadeDownPeriod = (currentSeconds >= fadeDownStartSeconds || currentSeconds < dimStartSeconds);
    }
  }
  
  bool inFadeUpPeriod = false;
  if (dimEndSeconds < fadeUpEndSeconds) {
    inFadeUpPeriod = (currentSeconds >= dimEndSeconds && currentSeconds < fadeUpEndSeconds);
  } else {
    inFadeUpPeriod = (currentSeconds >= dimEndSeconds || currentSeconds < fadeUpEndSeconds);
  }
  
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
      
      if (fadeElapsed >= fadeDurationMs) {
        currentMainBrightness = cfg.ledDimBrightness;
      } else {
        float progress = (float)fadeElapsed / (float)fadeDurationMs;
        int brightnessRange = cfg.ledBrightness - cfg.ledDimBrightness;
        currentMainBrightness = cfg.ledBrightness - (uint8_t)(brightnessRange * progress);
      }
      break;
    }
      
    case FADING_UP: {
      unsigned long fadeElapsed = millis() - fadeStartMillis;
      unsigned long fadeDurationMs = (unsigned long)cfg.ledDimFadeDuration * 1000;
      
      if (fadeElapsed >= fadeDurationMs) {
        currentMainBrightness = cfg.ledBrightness;
      } else {
        float progress = (float)fadeElapsed / (float)fadeDurationMs;
        int brightnessRange = cfg.ledBrightness - cfg.ledDimBrightness;
        currentMainBrightness = cfg.ledDimBrightness + (uint8_t)(brightnessRange * progress);
      }
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
  
  int dimStart = parseTimeToSeconds(cfg.ledDimStartTime.c_str());
  int dimEnd = parseTimeToSeconds(cfg.ledDimEndTime.c_str());
  
  if (dimStart == -1 || dimEnd == -1) {
    currentMainBrightness = cfg.ledBrightness;
    currentColonBrightness = ledBrightness;
    FastLED.setBrightness(ledBrightness);
    return;
  }
  
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
