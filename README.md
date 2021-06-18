# 7 Segment LED Clock

Arduino code for my beautiful 3D printed LED clock in a retro 7 segment display style.

You can find more details about the project and all downloadable files (STL, 3MF, STEP) [here](https://www.prusaprinters.org/prints/68013-7-segment-led-clock).

**As the current Prusa contest "Timekeepers" is still running, please consider to give the print a like over there. Thank you <3**

## Features

* Diplaying time (obviously)
  * Fancy rainbow effext or solid color
  * Time set by NTP
* Option to show the tempearature of a given location (Diplaying in °F is limited, as it can only display up to 99°F)
  * Teperature is shown in color (blue => cold, red => hot) as it cannot display negative values
* WiFi Manager for easy WLAN config
  * Multiple access point credentials
  * Double Reset Detection to start config portal after first configuration

## Installation

I highly recommend to use an ESP32 ([ESP32 DEVKITV1](https://www.aliexpress.com/item/4000152270368.html?spm=a2g0s.9042311.0.0.61af4c4dESOka0) / ESP-WROOM-32).

The code should also work with an ESP8266, but never tested it. It could be that the compiled sketch is too big for an ESP8266.

If not already done, install the ESP32 boards as described [here](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/).

Download the code of this repository (Code ==> Download ZIP), unzip it and remove the "-master" at the end of the directory name.

### Required libraries

Most libraries used can be installed using the Arduino IDE, but some have to be installed manually.

#### Library Manager

Install the following Libraries using the library manager in the Arduino IDE (Tools ==> Manage Libraries):

* FastLED
* ArduinoJson
* CronAlarms
* LittleFS_esp32
* ESP_DoubleResetDetector
* ESPAsync_WiFiManager (and all dependencies)

#### Manual Install

Click on each of the following links and download the most current version (Code ==> Download ZIP). Close the Ardiono IDE if open, unzip the files, remove the "-master" at the end of the directory name and move the folders into the Arduino IDE library folder (on MacOS it's located at "~/Documents/Arduino/libraries" by default)

* [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)



## Configuration

Open the file "7-Segment-LED-Clock.ino" in the Ardiono IDE, select the file "config_rename.h" and rename it to "config.h".

No changes in "config.h" are needed to get it up and running the first time.

If you want to show the temperature of a location as well, enable it by setting the variable "owmTempEnabled" to 1. You also have to set and set the API key, which you can get for free from [Open Weather Map](https://openweathermap.org/price), and your location ([city](https://openweathermap.org/current#name)).

Now compile and upload it to the ESP.

### WiFi

When starting up the clock the first time, it should show "ConF". This indicates that the configuration portal of the WiFiMaanger is active. Connect a device to the access point called "LED-Clock-Config" (default password is "ledclock") and configure your WiFi.

Once it's configured, you can start the configuration portal manually by resetting the ESP32 two times within few seconds. This can either be done by pressing the reset button on the ESP board twice or simply by plugging it out and in two times.

Connecting to the WiFi can take up to 40s, so be patient ;-)
