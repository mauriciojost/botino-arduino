# BOTINO

This is a cool-geek-fully-configurable alarm project.

![Botino](misc/images/botino-v0.jpg)

Features:

- Innovative design
- Setup via the internet / JSON
- Several configurable alarms
- Highly customizable routines: face expressions, LCD custom images, messages, LEDs, arms, fan, among others.
- Random quote of the day

## 1. Get It Started

### 1.1. Plug It

To get started first connect the module to any standard USB plug. Enjoy the HW test dancing.

### 1.2. Connect It to WIFI

Set up your phone's hotspot with the LCD-provided SSID and password. This way *botino* can access the internet for the first time. 

Then, using the provided setup-wifi script, set up the internet credentials you want your *botino* to use regularly. Botino will let you know if the WIFI set up went correctly. 

### 1.3. Play with It

Interaction with *Botino* is done via the Internet. 

You are the boss. You tell *Botino* what to do via HTTP queries. There are several settings you can tune. 

*Botino*'s internal components are called actors. Each actor has a precise role, and a set of properties
you can tune to get the behaviour you want.

#### Actor urls

Any actor can be read or writen via the internet, performing HTTP verbs on defined URLS.

Just replace `DEVICENAME` with your device name, and `ACTORNAME` with the name of the actor you want to read/write.

To get the current status of the actor: 

```
HTTP GET: http://dweet.io/get/latest/dweet/for/DEVICENAME-ACTORNAME-target
```

To change the current status of the actor (follow the same schema as observed with GET):

```
HTTP POST: http://dweet.io/get/latest/dweet/for/DEVICENAME-ACTORNAME-current
```

#### Actor: Settings

This actor is in charge of gather general purpose settings, mostly for development purposes. 

Properties: see [here for more information](src/actors/Settings.h)

#### Actor: Body

This actor is the core of the alarm. 

The most important concept is the routine. A routine is an move triggered at a specific moment. To define a routine it is
required to have a move (to perform when it's time) and a timing (to determine when to perform).

Properties: 

- `tX` timing X (X from 0 to 3): timing for the corresponding routine X (see the timing documentation)
- `mvX` move X (X from 0 to 3): move for the routine X, as a list of consecutive 3 letter-code for poses (see the poses documentation)
- `msgX` message X (X from 0 to 3): examples are "HI", "HELLO", they can be displayed using a given pose
- `imX` image X (X from 0 to 3): image in hexadecimal format, it can be displayed using a pose, see the image generator below for more information

Properties: see [here for more information](src/actors/Body.h)

[Image generator](https://docs.google.com/spreadsheets/d/1jXa9mFxeiN_bUji_WiCPKO_gB6pxQUeQ5QxgoSINqdc/edit#gid=0)


# 2. Other information

## Telnet

You can telnet the device for debugging purposes. You will get the logs via Wifi. 

You can also control the device. To do so you need to enter in configuration mode, by sending via telnet the command `conf` (and wait
for the device to pick it up). Then send the command `?` for help.

## Poses

A pose is a status of a device. For instance a LED on, a fan off, a message in the LCD, etc.

A sequence of poses make a move, which together with a timing make a routine. 

Properties: see [here for more information](src/actors/Body.h)

## Timing

The frequency at which a given actor will act depends on its timing configuration. It can normally be set up as any other property of the actor.

A timing is expressed as an integer value. It is possible to specify several types of timing: 

### Never

Simply `0` value. There will be no matching (the actor will not act).

### Concrete date-time

Simply use `1DDHHMMSS` (DD for days, HH for hours, MM for minutes, SS for seconds). 

For instance, if you want the actor to wake up at 15h00 any day, just set its frequency to `177150000` (the expression can be read as follows: 1 for concrete date-time mode, 77 for any day matching, 15 for matching only at 15h, 00 for matching only at 00m, and 00 for matching only at 00s).

### Frequency

Simply use `2DDHHMMSS`. 

The actor will act when the actual time component modulo the provided component equals zero. 

For example: if you want the actor to wake up every 2 hours, just set its frequency to `201026060` (the expression can be read as follows: 2 for frequency mode; 01 for any day, as any day number modulo 1 equals 0; 02 meaning every 2 hours, as any pair hour value modulo 2 equals 0, while odd hours will give 1, so no matching; 60 for matching only at 00m, and 00 for matching only at 00s).


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
platformio init --ide eclipse --board esp12e
```

Then open with _eclipse_.

```
ln -s `readlink -e .piolibdeps/Arduino/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.*` src/
```

## 3.3. Upload

To upload the firmware just do: 

```
export VERSION=`jshon -e version -u` && export PLATFORMIO_BUILD_FLAGS="'-D PROJ_VERSION=\"$VERSION\"'"

# then compile, upload, and display logs doing either:

platformio run --target upload # via serial port
 ./serial_monitor 0

# or:

platformio run --target upload --upload-port <IP> # OTA
telnet <IP>

```
