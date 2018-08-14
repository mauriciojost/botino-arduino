# BOTINO

[![Build Status](https://jenkins.martinenhome.com/buildStatus/icon?job=botino/master)](https://jenkins.martinenhome.com/job/botino/job/master/)

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

To get started first connect the module to any standard USB plug. Enjoy the HW test dancing.

### 1.2. Connect It to WIFI

Set up your smartphone's hotspot with the SSID and passwords provided by the LCD at startup. 
*Botino* will access it, and access the internet for the first time. This way you can set it up. 
This hotspot setup has to be done only once (per boot).

Then, using the provided [setup_device](setup_device) script, set up the wifi ssid/password you want your *botino* to use regularly (from your router / ISP). *Botino* will let you know if the WIFI set up went correctly. 

### 1.3. Play with It

Interaction with *Botino* is done via the Internet. You send the setup to the internet via HTTP queries, and *botino* regularly picks them up. 

You are the boss. You tell *Botino* what to do. There are several settings you can tune. 

*Botino*'s internal components are called actors. Each actor has a precise role, and a set of properties
you can tune to get the behaviour you want.

#### Actor urls

Any actor can be read or writen via the internet, performing HTTP verbs on defined URLS.

Just replace `DEVICENAME` with your device name, and `ACTORNAME` with the name of the actor you want to read/write.

| Purpose                       | HTTP VERB | URL                                                               |
| ----------------------------- | --------- | ----------------------------------------------------------------- |
| Get the information of an actor | GET       | http://dweet.io/get/latest/dweet/for/DEVICENAME-ACTORNAME-infos |
| Get the status of an actor    | GET       | http://dweet.io/get/latest/dweet/for/DEVICENAME-ACTORNAME-current |
| Change the status of an actor | POST      | http://dweet.io/dweet/for/DEVICENAME-ACTORNAME-target             |


To change the current status of the actor follow the same schema as observed with GET status.

#### Actors 

| Actor name    | Actor description                                                                    | Properties                         |
| ------------- | ------------------------------------------------------------------------------------ | ---------------------------------- |
| settings      | Is in charge of gather general purpose settings, mostly for development purposes.    | [Settings.h](src/actors/Settings.h)|
| body          | This is the core of the alarm, driven by routines triggered at specific moments.     | [Body.h](src/actors/Body.h)        |
| images        | Holds custom images to be used.                                                      | [Images.h](src/actors/Images.h)    |
| messages      | Holds custom messages to be used.                                                    | [Messages.h](src/actors/Messages.h)|
| ...           |                                                                                      | [...](src/actors/)                 |

# 2. Extras

## Telnet

You can telnet the device for debugging purposes. You will get the logs via Wifi. 

You can also control the device. To do so you need to enter in configuration mode, by sending via telnet the command `conf` (and wait
for the device to pick it up). Then send the command `help` for help.

## Poses

A pose is a status of a device. For instance a LED on, a fan off, a message in the LCD, etc.

A sequence of poses make a move, which together with a timing make a routine. 

Properties: see [here for more information](src/actors/Body.h)

## Timing

The frequency at which a given actor will act depends on its timing configuration. It can normally be set up as any other property of the actor.

A timing is expressed as an integer value. It is possible to specify several types of timing: 

| Timing type       | Description                                                  | Format       | Example                                |
| ----------------- | ------------------------------------------------------------ |:-------------|:--------------------------------------:|
| never             | No matching ever.                                            | `0`          | `0`                                    |
| day-time          | Match a custom day-time (within a month).                    | `1DDHHMMSS` | `177050000` (any day, 05h00m00s)        |
| modulo-frequency  | Match a component-modulo expression.                         | `2DDHHMMSS` | `201013060` (any day, every 30 minutes) |
| period secs       | Match every given seconds period.                            | `3xxxxxxxx` | `300000099` (every 99 seconds)          |
| one-off           | Match only once, then come back to previous setting          | `4xxxxxxxx` | `400000000` (match once, then as before)|

### Never

Simply `0` value. There will be no matching (the actor will not act).

### Concrete day-time

Simply use `1DDHHMMSS` (DD for days, HH for hours, MM for minutes, SS for seconds). 

For instance, if you want the actor to wake up at 15h00 any day, just set its frequency to `177150000` (the expression can be read as follows: 1 for concrete date-time mode, 77 for any day matching, 15 for matching only at 15h, 00 for matching only at 00m, and 00 for matching only at 00s).

### Modulo/Frequency

Simply use `2DDHHMMSS`. 

The actor will act when the actual time component modulo the provided component equals zero. 

For example: if you want the actor to wake up every 2 hours, just set its frequency to `201026060` (the expression can be read as follows: 2 for frequency mode; 01 for any day, as any day number modulo 1 equals 0; 02 meaning every 2 hours, as any pair hour value modulo 2 equals 0, while odd hours will give 1, so no matching; 60 for matching only at 00m, and 00 for matching only at 00s).

### Period Seconds

Simply use `3xxxxxxxx`. 

The actor will act with the period given as argument (in the `xxx...`).

For example: if you want the actor to wake up every 999 seconds, just set its frequency to `300000999`.

### One off

Simply use `4xxxxxxxx`. 

The actor will act immediately. 

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
export VERSION=`cat library.json | jshon -e version -u`
# or
export VERSION=$(git rev-parse --short HEAD) 

# then compile, upload, and display logs doing either:

export PLATFORMIO_BUILD_FLAGS="`cat profiles/demo.prof`" 

platformio run --target upload 
# or:
platformio run --target upload --upload-port <IP> # OTA

 ./serial_monitor 0


```

To upload the `data` folder just do: 

```
PLATFORMIO_BUILD_FLAGS="`cat profiles/demo.prof`" platformio run --target buildfs
PLATFORMIO_BUILD_FLAGS="`cat profiles/demo.prof`" platformio run --target uploadfs
```
