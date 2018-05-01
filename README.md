# README

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
