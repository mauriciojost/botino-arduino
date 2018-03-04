# README

Board used: ESP-01

http://www.esp8266.com/wiki/doku.php?id=esp8266-module-family

## Initialization for test

```
ln -s `readlink -e libs/main4ino-arduino/src/*` src
ln -s `readlink -e libs/log4ino-arduino/src/*` src
pio lib install "LiquidCrystal"
```

Was having issues with log4ino as dependency.
At the end the dependencies declared in `platformio.ini` can be replaced by 
the `ln` commands above for any dependency.

## IDE

```
platformio init --ide eclipse --board <ID>
```

For example: 

```
platformio init --ide eclipse --board esp01_1m
```

Then open with _eclipse_.

```
ln -s `readlink -e .piolibdeps/Arduino/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.*` src/
```
