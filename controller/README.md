# build-a-vent : Controller

This subtree contains the hard- and software of the controller core of build-a-vent

We are developing the build-a-vent controller based on the 
[Arduino](https://en.wikipedia.org/wiki/Arduino) IDE.

Currently preferred CPU is the [ESP8266](https://en.wikipedia.org/wiki/ESP8266)
due to its WiFi-on-chip and the performance. 
Support for [ESP32](https://en.wikipedia.org/wiki/ESP32) is being prepared. 
All ESP8266 Platforms with enough free GPIO pins are supported. 

Reference Platform is the ESP8266-NodeMCU on ESP-12E
We probably won't support ESP-01 due to very limited I/O Ressources 
(one I2C-Bus, no free pins for 1-Wire thermosensors)

## Installation of Arduino IDE

[arduino.cc: Installguide on ](https://www.arduino.cc/en/Guide/HomePage) This guide covers
installation on all major development platforms.


## installation of ESP8266 for Arduino IDE

 * [instructables: Quick start to NodeMCU on Arduino](https://www.instructables.com/id/Quick-Start-to-Nodemcu-ESP8266-on-Arduino-IDE/)
 * [randomnerdtutorial: Generic ESP8266 install on Arduino ](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/)
 * [randomnerdtutorial: ESP32 installation on Arduino](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/) 
 * [heise: In german both for ESP8266 and ESP32](https://www.heise.de/ct/artikel/Arduino-IDE-installieren-und-fit-machen-fuer-ESP8266-und-ESP32-4130814.html)
 
## ESP32 platform

 * [circuits4you:ESP32-devkit IO description](https://circuits4you.com/2018/12/31/esp32-devkit-esp32-wroom-gpio-pinout/)
 
