# README

Board used: ESP-01

http://www.esp8266.com/wiki/doku.php?id=esp8266-module-family

```
git clone --recursive git@bitbucket.org:mauriciojost/arduino-wifi.git
cd arduino-wifi
ln -s `readlink -e libs/log4ino-arduino/src/log4ino` src/
ln -s `readlink -e libs/main4ino-arduino/src/main4ino` src/

```

## IDE

```
platformio init --ide eclipse --board <ID>
```

For example: 

```
platformio init --ide eclipse --board esp01_1m
```

Then open with _eclipse_.
