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
           httpPost,
           httpGet,
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
           apiDevicePass);

  m->getModule()->setDescription(VERSION_DESCRIPTION_JSON);

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
  }
}

void loop() {
  m->loop();
}
