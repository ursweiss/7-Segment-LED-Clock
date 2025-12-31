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
      "label": "Weather Settings",
      "icon": "cloud",
      "fields": [
        {
          "id": "weatherTempEnabled",
          "type": "checkbox",
          "label": "Show Temperature",
          "help": "Display temperature periodically",
          "default": 1,
          "applyMethod": "instant"
        },
        {
          "id": "locationLatitude",
          "type": "text",
          "label": "Latitude",
          "help": "Decimal degrees (-90 to 90). Use 'Detect My Location' button or find at open-meteo.com",
          "default": "",
          "validation": {
            "required": false,
            "pattern": "^-?([0-9]|[1-8][0-9]|90)(\\.[0-9]+)?$"
          },
          "showIf": {"field": "weatherTempEnabled", "equals": 1},
          "applyMethod": "restart"
        },
        {
          "id": "locationLongitude",
          "type": "text",
          "label": "Longitude",
          "help": "Decimal degrees (-180 to 180). Use 'Detect My Location' button or find at open-meteo.com",
          "default": "",
          "validation": {
            "required": false,
            "pattern": "^-?([0-9]|[1-9][0-9]|1[0-7][0-9]|180)(\\.[0-9]+)?$"
          },
          "showIf": {"field": "weatherTempEnabled", "equals": 1},
          "applyMethod": "restart"
        },
        {
          "id": "locationUnits",
          "type": "select",
          "label": "Units",
          "help": "Temperature units to display",
          "default": "metric",
          "options": [
            {"value": "metric", "label": "Metric (°C)"},
            {"value": "imperial", "label": "Imperial (°F)"}
          ],
          "showIf": {"field": "weatherTempEnabled", "equals": 1},
          "applyMethod": "restart"
        },
        {
          "id": "weatherTempDisplayTime",
          "type": "number",
          "label": "Display Duration",
          "help": "Seconds to show temperature",
          "default": 5,
          "validation": {
            "min": 1,
            "max": 60
          },
          "showIf": {"field": "weatherTempEnabled", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "weatherTempMin",
          "type": "number",
          "label": "Min Temperature",
          "help": "Minimum for color gradient (blue)",
          "default": -40,
          "validation": {
            "min": -99,
            "max": 99
          },
          "showIf": {"field": "weatherTempEnabled", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "weatherTempMax",
          "type": "number",
          "label": "Max Temperature",
          "help": "Maximum for color gradient (red)",
          "default": 50,
          "validation": {
            "min": -99,
            "max": 99
          },
          "showIf": {"field": "weatherTempEnabled", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "weatherTempSchedule",
          "type": "text",
          "label": "Display Schedule",
          "help": "Cron format: sec min hour day month dow",
          "default": "30 * * * * *",
          "validation": {
            "required": true,
            "pattern": "^[\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+$"
          },
          "showIf": {"field": "weatherTempEnabled", "equals": 1},
          "applyMethod": "instant"
        },
        {
          "id": "weatherUpdateSchedule",
          "type": "text",
          "label": "Update Schedule",
          "help": "Cron format: when to fetch weather",
          "default": "0 5 * * * *",
          "validation": {
            "required": true,
            "pattern": "^[\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+ [\\\\d*,/-]+$"
          },
          "showIf": {"field": "weatherTempEnabled", "equals": 1},
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
