# BOTINO

This is a cool-geek-fully-configurable alarm project.

Features:
- WIFI setup
- Setup via the internet / JSON
- Clock display
- Several messages
- Several moves, which are a sequence of poses.
- Several poses available, including among others:
  - A given face expression (sad, smile, sleepy, ...)
  - A given LCD message from the ones above
  - A LCD display inversion
  - A current-time display
  - 3 individually controlled LEDs
  - 2 individually controlled 10-position arms
  - 1 fan
  - Delays from 1 to 9 seconds
- Several routines, each containing a frequency/time of triggering, and a move to perform
- A button that triggers a move

To get started first connect the module to any standard USB plug. Enjoy the HW test dancing.

### WIFI network setup

Set up your phone's hotspot with the LCD-provided SSID and password. This way *botino* can access the internet for the first time. 

Then, using the provided setup-wifi script, set up the internet credentials you want your *botino* to use regularly. Botino will let you know if the WIFI set up went correctly. 

### Botino monitoring & setup

#### 

Using the name of your botino, write to the following URLs to change its behavior:

http://dweet.io/get/latest/dweet/for/DEVICENAME-settings-target
http://dweet.io/get/latest/dweet/for/DEVICENAME-body-target

http://dweet.io/get/latest/dweet/for/DEVICENAME-settings-current
http://dweet.io/get/latest/dweet/for/DEVICENAME-body-current


# Other information
Board used: NODEMCU / ESP-01

http://www.esp8266.com/wiki/doku.php?id=esp8266-module-family

## Initialization for test

```
./pull_dependencies dependencies.conf
```

If the LCD is displaying using a too big size font, you can change it in:

```
src/Adafruit_SSD1306.h # uncomment 64 instead of 32
```

## IDE

To get started with _eclipse_ do:
```
platformio init --ide eclipse --board esp01_1m
```

Then open with _eclipse_.

```
ln -s `readlink -e .piolibdeps/Arduino/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.*` src/
```
