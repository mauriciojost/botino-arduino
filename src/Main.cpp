/**
 * This file contains:
 * - entry point for arduino programs (setup and loop functions)
 * - declaration of HW specific functions (the definition is in a dedicated file)
 * - other functions that are not defined by HW specific but that use them, that are required by the module
 *   (so that it can be passed as callback).
 * The rest should be put in Module so that they can be tested regardless of the HW used behind.
 */

#include <Main.h>

ModuleBotino *m;

#ifdef ARDUINO // on ESP8266
#include <MainESP8266.h>
#else // on PC
#include <MainX86_64.h>
#endif // ARDUINO


#define VERSION_DESCRIPTION_JSON "{"\
	"\"version\":\"" STRINGIFY(PROJ_VERSION) "\","\
	"\"json\":\"["\
   "{\\\"patterns\\\": [\\\"^actor1.p1$\\\"], \\\"descriptions\\\": [\\\"Property 1\\\"], \\\"examples\\\": [\\\"1\\\", \\\"2\\\"]}"\
	 "]\"}"

bool initWifiSimple() {
  Settings *s = m->getModuleSettings();
  log(CLASS_MAIN, Info, "W.steady");
  bool connected = initWifi(s->getSsid(), s->getPass(), true, 10);
  return connected;
}

void setup() {
  m = new ModuleBotino();
  m->setup(setupArchitecture,
           lcdImg,
           arms,
           messageFunc,
           ios,
           initWifiSimple,
           stopWifi,
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
           updateFirmware,
           testArchitecture,
           apiDeviceLogin,
           apiDevicePass,
           NULL
					 );
  ModuleStartupPropertiesCode ec = m->startupProperties();

  initWifiSimple();
  logRaw(CLASS_MAIN, Info, VERSION_DESCRIPTION_JSON);
  m->getModule()->getPropSync()->pushDescription(VERSION_DESCRIPTION_JSON);
  if (ec != ModuleStartupPropertiesCodeSuccess) {
    log(CLASS_MAIN, Error, "Failure: %d", (int)ec);
    abort("Could not startup");
  }
}

void loop() {
  m->loop();
}
