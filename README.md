# README

Board used: ESP-01

http://www.esp8266.com/wiki/doku.php?id=esp8266-module-family

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
