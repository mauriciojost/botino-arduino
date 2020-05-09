#include <main4ino/Misc.h>
#include <ModuleBotino.h>
#include <Platform.h>
#include <Constants.h>

#ifndef PROJ_VERSION
#define PROJ_VERSION "snapshot"
#endif // PROJ_VERSION

#define CLASS_MAIN "MA"

//////////////////////////////////////////////////////////////
// Provided by generic Main
//////////////////////////////////////////////////////////////

// Standard arduino setup 


void setup() {
  setupArchitecture();
  m = new ModuleBotino();
  m->setup(lcdImg,
           arms,
           messageFunc,
           ios,
           initWifiSimple,
           nop,
           httpMethod,
           clearDevice,
           readFile,
           writeFile,
           sleepInterruptable,
           deepSleepNotInterruptable,
           configureModeArchitecture,
           runModeArchitecture,
           commandArchitecture,
           infoArchitecture,
           updateFirmwareVersion,
           testArchitecture,
           apiDeviceLogin,
           apiDevicePass,
           getLogBuffer,
           buttonIsPressed
           );

  log(CLASS_MAIN, Info, "Startup of properties");
#ifdef BIMBY_MODE
  StartupStatus c = m->startupPropertiesLight();
#else // BIMBY_MODE
  StartupStatus c = m->startupProperties();
#endif // BIMBY_MODE
  m->getBot()->setMode(c.botMode);
  if (c.startupCode != ModuleStartupPropertiesCodeSuccess) {
    log(CLASS_MAIN, Error, "Failure: %d", (int)c.startupCode);
    bool i = sleepInterruptable(now(), 10);
    if (i) {
    } else {
      abort("Could not startup");
    }
  } else {
#ifdef ARDUINO
    // Tricky description pushing: unless declared specially (with PROGMEM for instance)
    // constant variables are kept in RAM. This is a 4K object that cannot stay there.
    // https://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html
    // Solution: put it in PROGMEM and store it in RAM only when required to push.
#include <Description.json.h>
#ifndef BIMBY_MODE
    String* desc = new String(DESCRIPTION_JSON_VERSION);
    log(CLASS_MAIN, Debug, "Pushing description...");
    m->getModule()->getPropSync()->pushDescription(desc->c_str());
    delete desc;
#endif // BIMBY_MODE
#endif // ARDUINO
  }
}

void loop() {
  m->loop();
}

