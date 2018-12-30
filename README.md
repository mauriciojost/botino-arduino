# BOTINO

[![Build Status](https://jenkins.martinenhome.com/buildStatus/icon?job=botino-arduino/master)](https://jenkins.martinenhome.com/job/botino-arduino/job/master/)

This is a cool-geek-fully-configurable alarm project.

![Botino](misc/images/botino-v0.jpg)

What it does:

- It connects to Wifi
- It gets set up via REST JSON
- It offers several moves (smily arms up, greetings, dances, disco night, rainy, etc.)
- It offers severall alarms (can invoke moves)
- It offers moves using: LCD, mobile arms, messages, LEDs, fan, IFTTT service
- Random quote of the day
- Random future reading
- IFTTT connectivity (as This component for now)

## 1. Get Started

### 1.1. Plug It

To get started first connect the module to any standard USB plug.

### 1.2. Set up you WIFI connection

Connect to it via serial port (TODO explain why) and set up your wifi settings via commands (use `help` to get started).

### 1.3. Play with It

Interaction with *Botino* is done via the Internet. You send the setup to the internet via HTTP queries, and *Botino* regularly picks them up. 

You are the boss. You tell *Botino* what to do. There are several settings you can tune. 

*Botino*'s internal components are called actors. Each actor has a precise role, and a set of properties
you can tune to get the behaviour you want.


| Actor name    | Actor description                                                                    | Properties                         |
| ------------- | ------------------------------------------------------------------------------------ | ---------------------------------- |
| settings      | Is in charge of gather general purpose settings, mostly for development purposes.    | [Settings.h](src/actors/Settings.h)|
| body          | This is the core of the alarm, driven by routines triggered at specific moments.     | [Body.h](src/actors/Body.h)        |
| images        | Holds custom images to be used.                                                      | [Images.h](src/actors/Images.h)    |
| ...           |                                                                                      | [...](src/actors/)                 |

Control your botino via the [Main4ino portal](http://martinenhome.com:6780).

# 2. Extras

## Telnet

You can telnet the device for debugging purposes. You will get the logs via Wifi. Debug mode must be enabled.

You can also control the device. To do so you need to enter in configuration mode, by sending via telnet the command `conf` (and wait
for the device to pick it up). Then send the command `help` for help.

## Poses

A pose is a status of a device. For instance a LED on, a fan off, a message in the LCD, etc.

A sequence of poses make a move, which together with a timing make a routine. 

Properties: see [here for more information](src/actors/Body.h)

## Timing

Refer to the `Timing` section of the documentation of `main4ino`.


# 3. Contribute

## 3.1. Hardware

For information, the Board used is [NODEMCU / ESP-01](http://www.esp8266.com/wiki/doku.php?id=esp8266-module-family).

- ESP8266 ESP-12E (see a full [list of variants here](https://www.esp8266.com/wiki/doku.php?id=esp8266-module-family))


## 3.2. Software

To prepare your development environment first do:

```
./pull_dependencies
```

The project is a `platformio` project.

### Heads up

When contributing, keep always in mind the best practices: 

- Try not to overuse the heap (only 4K!): prefer static memory allocation rathenr than dynamic one
- Reuse instances as much as possible

### Eclipse

To get started with _eclipse_ do:
```
platformio init --ide eclipse --board esp12e
```

Then open with _eclipse_. When missing sources, you could link them to the `src` directory as follows:

```
ln -s `readlink -e .piolibdeps/Arduino/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.*` src/
```
However this method is not recommended.

## 3.3. Upload

To upload the firmware just do: 

```
# upload via USB serial port:

 ./upload profiles/dev7-custom.prof

# if you want to upload via OTA: 

 platformio run --target upload --upload-port <IP> # OTA

```

To see the logs:
```
# using minicom:
 sudo minicom -D /dev/ttyUSB0 -b 128000 -z -L -l # + Ctrl+a u  (and Ctrl+a q to exit)

# using platformio serial monitor:
 ./serial_monitor 0

```

To upload the `data` folder just do: 

```
PLATFORMIO_BUILD_FLAGS="`cat profiles/demo.prof`" platformio run --target buildfs
PLATFORMIO_BUILD_FLAGS="`cat profiles/demo.prof`" platformio run --target uploadfs
```

## 3.4. Simulate

Even if the current main implementation uses ESP8266, *Botino* is meant to be multi platform. You can launch it even in your PC:

```
./simulate 0 100 # mode interactive, 100 steps and quit
```

## 3.5. Test

```
PLATFORMIO_BUILD_FLAGS="`cat profiles/test.prof`" ./launch_tests
```
