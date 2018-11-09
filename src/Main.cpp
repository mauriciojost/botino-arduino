/**
 * This file aims to be the only HW specific source code file in the whole project.
 * The rest of the classes (and the 100% of their code) should be testeable without need
 * of Arduino specific HW.
 */

#include <Main.h>

Module m;

#ifndef SIMULATE // on ESP8266
#include <MainESP8266.h>
#else // on PC
#include <MainX86_64.h>
#endif // SIMULATE

bool initWifiSteady() {
  SetupSync *s = m.getSetupSync();
  if (s->isInitialized()) {
    const char *wifiSsid = s->getSsid();
    const char *wifiPass = s->getPass();
    log(CLASS_MAIN, Info, "W.steady");
    bool connected = initWifi(wifiSsid, wifiPass, true, 10);
    return connected;
  } else {
    log(CLASS_MAIN, Info, "W.steady null");
    return false;
  }
}

void setup() {

  m.setup(lcdImg, arms, messageFunc, ios, initWifiSteady, httpPost, httpGet, clearDevice, readFile, writeFile);

  setupArchitecture();

  m.actall();
}

void configureMode() {
  unsigned long cycleBegin = millis();
  configureModeArchitecture();
  sleepInterruptable(cycleBegin, PERIOD_CONFIGURE_MSEC);
}

void runMode() {
  unsigned long cycleBegin = millis();
  log(CLASS_MAIN, Info, "Version: %s", STRINGIFY(PROJ_VERSION));

  // Handle keys
  SetupSync *ss = m.getSetupSync();
  if (ss->isInitialized()) {
    m.getIfttt()->setKey(ss->getIfttt());
    m.getClockSync()->setDbKey(ss->getTimeKey());
  }

  runModeArchitecture();

  m.loop(false, false, true);
  sleepInterruptable(cycleBegin, PERIOD_MSEC);
}

void loop() {
  switch (m.getBot()->getMode()) {
    case (RunMode):
      runMode();
      break;
    case (ConfigureMode):
      configureMode();
      break;
    default:
      break;
  }
}
