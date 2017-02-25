# README

Board used: ESP-01

http://www.esp8266.com/wiki/doku.php?id=esp8266-module-family

```
git clone --recursive git@bitbucket.org:mauriciojost/arduino-wifi.git
cd arduino-wifi
ln -s `readlink -e libs/log4ino-arduino/src/log4ino` src
ln -s `readlink -e libs/main4ino-arduino/src/main4ino` src

```

## IDE

- Open using CLion or IntelliJ v14 (with C++ support).
- Mark `src` directory as `Sources Root`.
- In `Project Settings` -> `C++` add any `Additional include directory` (will modify `misc.xml` so that an include will be added and can be used as a reference to add the other ones).
- Use the script in `misc` to add other external includes.
