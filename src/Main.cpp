/**
 * This file aims to be very short.
 * It can only contain functions that are composition of functions provided by architecture functions (so
 * that it can be passed as callback). The rest should be put in Module.
 */

#include <Main.h>

Module m;

#ifdef ARDUINO // on ESP8266
#include <MainESP8266.h>
#else   // on PC
#include <MainX86_64.h>
#endif // ARDUINO

bool initWifiSimple() {
  Settings *s = m.getSettings();
  log(CLASS_MAIN, Info, "W.steady");
  bool connected = initWifi(s->getSsid(), s->getPass(), true, 10);
  return connected;
}

void setup() {
  m.setup(
  		setupArchitecture,
			lcdImg,
			arms,
			messageFunc,
			ios,
			initWifiSimple,
			httpPost,
			httpGet,
			clearDevice,
			readFile,
			writeFile,
			abort,
			sleepInterruptable,
			configureModeArchitecture,
			runModeArchitecture
			);
}

void loop() {
	m.loop();
}

