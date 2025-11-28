#ifndef SCHEMA_H
#define SCHEMA_H

#include <Arduino.h>

const char SCHEMA_JSON[] PROGMEM = R"===(
{
  "groups": [
    {
      "id": "wifi",
      "label": "WiFi Settings",
      "icon": "wifi",
      "fields": [
        {
          "id": "portalSsid",
          "type": "text",
          "label": "Config Portal SSID",
          "help": "Name of WiFi access point when config portal is active",
          "default": "LED-Clock-Config",
          "validation": {
            "required": true,
            "minLength": 1,
            "maxLength": 32
          }
        },
        {
          "id": "portalPassword",
          "type": "password",
          "label": "Config Portal Password",
          "help": "Password required to access config portal",
          "default": "ledclock",
          "validation": {
            "required": true,
            "minLength": 8,
            "maxLength": 63
          }
        }
      ]
    },
    {
      "id": "clock",
      "label": "Clock Display",
      "icon": "clock",
      "fields": [
        {
          "id": "clockColorMode",
          "type": "select",
          "label": "Color Mode",
          "help": "How to color the LED segments",
          "default": 1,
          "options": [
            {"value": 0, "label": "Solid Color"},
            {"value": 1, "label": "Color Palette"}
          ],
          "applyMethod": "instant"
        },
        {
          "id": "clockColorSolid",
          "type": "color",
          "label": "Solid Color",
          "help": "Color when using Solid Color mode (RGB hex)",
          "default": "#00FF00",
          "showIf": {"field": "clockColorMode", "equals": 0},
          "applyMethod": "instant"
        },
        {
          "id": "clockColorPaletteIndex",
          "type": "select",
          "label": "Color Palette",
          "help": "Predefined color palette to use",
          "default": 0,
          "options": [
            {"value": 0, "label": "Rainbow"},
            {"value": 1, "label": "Cloud"},
            {"value": 2, "label": "Lava"},
            {"value": 3, "label": "Ocean"},
            {"value": 4, "label": "Forest"}
          ],
          "showIf": {"field": "clockColorMode", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "clockColorCharBlend",
          "type": "number",
          "label": "Character Blend",
          "help": "Amount to blend colors between characters (0=disabled)",
          "default": 5,
          "validation": {
            "min": 0,
            "max": 255
          },
          "showIf": {"field": "clockColorMode", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "clockColorBlending",
          "type": "select",
          "label": "Blend Type",
          "help": "How to blend palette colors",
          "default": 1,
          "options": [
            {"value": 0, "label": "No Blend"},
            {"value": 1, "label": "Linear Blend"}
          ],
          "showIf": {"field": "clockColorMode", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "clockSecIndicatorDiff",
          "type": "number",
          "label": "Second Indicator Dim",
          "help": "How much to darken second indicator (0=disabled)",
          "default": 32,
          "validation": {
            "min": 0,
            "max": 255
          },
          "applyMethod": "instant"
        }
      ]
    },
    {
      "id": "weather",
      "label": "Weather & Temperature",
      "icon": "cloud",
      "fields": [
        {
          "id": "owmApiServer",
          "type": "text",
          "label": "API Server",
          "help": "OpenWeatherMap API server hostname",
          "default": "api.openweathermap.org",
          "validation": {
            "required": true
          },
          "applyMethod": "restart"
        },
        {
          "id": "owmApiKey",
          "type": "password",
          "label": "API Key",
          "help": "Your OpenWeatherMap API key (free from openweathermap.org)",
          "default": "",
          "validation": {
            "required": true,
            "minLength": 32,
            "maxLength": 32
          },
          "applyMethod": "restart"
        },
        {
          "id": "owmLocation",
          "type": "text",
          "label": "Location",
          "help": "City and country code (e.g., Nürensdorf,CH)",
          "default": "",
          "validation": {
            "required": true
          },
          "applyMethod": "restart"
        },
        {
          "id": "owmUnits",
          "type": "select",
          "label": "Units",
          "help": "Temperature units to display",
          "default": "metric",
          "options": [
            {"value": "metric", "label": "Metric (°C)"},
            {"value": "imperial", "label": "Imperial (°F)"}
          ],
          "applyMethod": "restart"
        },
        {
          "id": "owmTempEnabled",
          "type": "checkbox",
          "label": "Show Temperature",
          "help": "Display temperature periodically",
          "default": 1,
          "applyMethod": "instant"
        },
        {
          "id": "owmTempDisplayTime",
          "type": "number",
          "label": "Display Duration",
          "help": "Seconds to show temperature",
          "default": 5,
          "validation": {
            "min": 1,
            "max": 60
          },
          "showIf": {"field": "owmTempEnabled", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "owmTempMin",
          "type": "number",
          "label": "Min Temperature",
          "help": "Minimum for color gradient (blue)",
          "default": -40,
          "validation": {
            "min": -99,
            "max": 99
          },
          "applyMethod": "instant"
        },
        {
          "id": "owmTempMax",
          "type": "number",
          "label": "Max Temperature",
          "help": "Maximum for color gradient (red)",
          "default": 50,
          "validation": {
            "min": -99,
            "max": 99
          },
          "applyMethod": "instant"
        },
        {
          "id": "owmTempSchedule",
          "type": "text",
          "label": "Display Schedule",
          "help": "Cron format: sec min hour day month dow",
          "default": "30 * * * * *",
          "validation": {
            "required": true,
            "pattern": "^[\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+$"
          },
          "applyMethod": "instant"
        },
        {
          "id": "owmUpdateSchedule",
          "type": "text",
          "label": "Update Schedule",
          "help": "Cron format: when to fetch weather",
          "default": "0 5 * * * *",
          "validation": {
            "required": true,
            "pattern": "^[\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+$"
          },
          "applyMethod": "instant"
        }
      ]
    },
    {
      "id": "led",
      "label": "LED Brightness",
      "icon": "lightbulb",
      "fields": [
        {
          "id": "ledBrightness",
          "type": "number",
          "label": "Maximum Brightness",
          "help": "LED brightness level (0-255)",
          "default": 128,
          "validation": {
            "min": 0,
            "max": 255
          },
          "applyMethod": "instant"
        },
        {
          "id": "ledDimEnabled",
          "type": "checkbox",
          "label": "Enable Auto Dimming",
          "help": "Automatically dim at night",
          "default": 1,
          "applyMethod": "instant"
        },
        {
          "id": "ledDimBrightness",
          "type": "number",
          "label": "Dimmed Brightness",
          "help": "Brightness when dimmed (0-255)",
          "default": 64,
          "validation": {
            "min": 0,
            "max": 255
          },
          "showIf": {"field": "ledDimEnabled", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "ledDimFadeDuration",
          "type": "number",
          "label": "Fade Duration",
          "help": "Seconds for brightness transition",
          "default": 30,
          "validation": {
            "min": 1,
            "max": 300
          },
          "showIf": {"field": "ledDimEnabled", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "ledDimStartTime",
          "type": "time",
          "label": "Dim Start Time",
          "help": "When to start dimming (HH:MM)",
          "default": "22:00",
          "validation": {
            "required": true,
            "pattern": "^([01]\\\\d|2[0-3]):[0-5]\\\\d$"
          },
          "showIf": {"field": "ledDimEnabled", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "ledDimEndTime",
          "type": "time",
          "label": "Dim End Time",
          "help": "When to end dimming (HH:MM)",
          "default": "06:00",
          "validation": {
            "required": true,
            "pattern": "^([01]\\\\d|2[0-3]):[0-5]\\\\d$"
          },
          "showIf": {"field": "ledDimEnabled", "equals": 1},
          "applyMethod": "instant"
        }
      ]
    },
    {
      "id": "advanced",
      "label": "Advanced",
      "icon": "settings",
      "collapsed": true,
      "fields": [
        {
          "id": "clockUpdateSchedule",
          "type": "text",
          "label": "Clock Update Schedule",
          "help": "Cron format: when to update display",
          "default": "* * * * * *",
          "validation": {
            "required": true,
            "pattern": "^[\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+$"
          },
          "applyMethod": "restart"
        }
      ]
    }
  ]
}
)===";

#endif // SCHEMA_H
