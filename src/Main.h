#ifndef MAIN_INC
#define MAIN_INC

#define CLASS_MAIN "MA"

#include <Constants.h>
#include <ModuleBotino.h>
#include <Platform.h>
#include <main4ino/Misc.h>

#ifdef ARDUINO

#ifdef ESP8266 // on ESP8266
#include <PlatformESP8266.h>
#endif // ESP8266

#ifdef ESP32 // on ESP8266
#include <PlatformESP32.h>
#endif // ESP8266

#else // on PC
#include <PlatformX86_64.h>
#endif // ARDUINO

#ifndef PROJ_VERSION
#define PROJ_VERSION "snapshot"
#endif // PROJ_VERSION

//////////////////////////////////////////////////////////////
// Provided by generic Main
//////////////////////////////////////////////////////////////

// Standard arduino setup
void setup();

// Standard arduino loop
void loop();

bool initWifiSimple();

#endif // MAIN_INC
