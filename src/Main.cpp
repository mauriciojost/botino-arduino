#include <Main.h>


void setup() {
  m = new ModuleBotino();
  m->setup(setupArchitecture,
           lcdImg,
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
           getLogBuffer
           );

  log(CLASS_MAIN, Info, "Startup of properties");
  ModuleStartupPropertiesCode ec = m->startupProperties();


  if (ec != ModuleStartupPropertiesCodeSuccess) {
    log(CLASS_MAIN, Error, "Failure: %d", (int)ec);
    bool i = sleepInterruptable(now(), 10);
    if (i) {
      m->getBot()->setMode(ConfigureMode);
    } else {
      abort("Could not startup");
    }
  } else {
    // Tricky description pushing: unless declared specially (with PROGMEM for instance)
    // constant variables are kept in RAM. This is a 4K object that cannot stay there.
    // https://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html
    // Solution: put it in PROGMEM and store it in RAM only when required to push.
#include <Description.json.h>
    String desc = String(DESCRIPTION_JSON_VERSION);
    log(CLASS_MAIN, Debug, "Pushing description...");
    logRaw(CLASS_MAIN, Debug, desc.c_str());
    m->getModule()->getPropSync()->pushDescription(desc.c_str());
  }
}

void loop() {
  m->loop();
}
