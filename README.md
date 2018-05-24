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
  - Several given custom LCD images 
  - A given LCD message from the ones above
  - A LCD display inversion
  - A current-time display
  - 3 individually controlled LEDs
  - 2 individually controlled 10-position arms
  - 1 fan
  - Delays from 1 to 9 seconds
- Several routines, each containing a frequency/time of triggering, and a move to perform
- A button that triggers a move

## 1. Get It Started

### 1.1. Plug It

To get started first connect the module to any standard USB plug. Enjoy the HW test dancing.

### 1.2. Connect It to WIFI

Set up your phone's hotspot with the LCD-provided SSID and password. This way *botino* can access the internet for the first time. 

Then, using the provided setup-wifi script, set up the internet credentials you want your *botino* to use regularly. Botino will let you know if the WIFI set up went correctly. 

### 1.3. Play with It

Interaction with *Botino* is done via the Internet. 

You are the boss. You tell *Botino* what to do via HTTP queries. There are several settings you can tune. 

#### General Settings

Using the name of your botino, write to the following URLs to change its behavior:

HTTP GET: http://dweet.io/get/latest/dweet/for/DEVICENAME-settings-target
HTTP POST: http://dweet.io/get/latest/dweet/for/DEVICENAME-settings-current

#### Body Settings

Using the name of your botino, write to the following URLs to change its behavior:

HTTP GET: http://dweet.io/get/latest/dweet/for/DEVICENAME-body-current
HTTP POST: http://dweet.io/get/latest/dweet/for/DEVICENAME-body-target


# 2. Other information

## Timing

Timing

# 3. Contribute

## 3.1. Hardware

For information, the Board used is [NODEMCU / ESP-01](http://www.esp8266.com/wiki/doku.php?id=esp8266-module-family).

## 3.2. Software

To prepare your development environment first do:

```
./pull_dependencies dependencies.conf
```

The project is a `platformio` project.

### Eclipse

To get started with _eclipse_ do:
```
platformio init --ide eclipse --board esp01_1m
```

Then open with _eclipse_.

```
ln -s `readlink -e .piolibdeps/Arduino/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.*` src/
```
