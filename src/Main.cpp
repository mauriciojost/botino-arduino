/**
 * This file contains:
 * - entry point for arduino programs (setup and loop functions)
 * - declaration of HW specific functions (the definition is in a dedicated file)
 * - other functions that are not defined by HW specific but that use them, that are required by the module
 *   (so that it can be passed as callback).
 * The rest should be put in Module so that they can be tested regardless of the HW used behind.
 */

#include <Main.h>

#define WIFI_CONNECTION_RETRIES 6

#include <Description.json.h>

void nop() {}

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
