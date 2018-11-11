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
  Settings *s = m.getSettings();
  log(CLASS_MAIN, Info, "W.steady");
  bool connected = initWifi(s->getSsid(), s->getPass(), true, 10);
  return connected;
}

void setup() {
  Buffer timeAux(19);
  log(CLASS_MAIN, Info, "# Setup module...");
  m.setup(lcdImg, arms, messageFunc, ios, initWifiSteady, httpPost, httpGet, clearDevice, readFile, writeFile);
  log(CLASS_MAIN, Info, "# Setup architecture...");
  setupArchitecture();
  log(CLASS_MAIN, Info, "# Loading credentials stored in FS...");
  m.getPropSync()->fsLoadActorsProps(); // load mainly credentials already set
  log(CLASS_MAIN, Info, "# Syncing actors with server...");
  m.getPropSync()->serverSyncActors(); // sync properties from the server
  time_t leftTime = m.getBot()->getClock()->currentTime();

  log(CLASS_MAIN, Info, "# Previous actors' times: %s...", Timing::humanize(leftTime, &timeAux));
  m.getBot()->setActorsTime(leftTime);
  log(CLASS_MAIN, Info, "# Syncing clock...");

#ifdef DEEP_SLEEP_MODE
  log(CLASS_MAIN, Info, "Deep sleep...");
  bool block = true; // block clock afterwards, as its time must be saved and device restarted
#else
  bool block = false;
#endif
  m.getClockSync()->syncClock(block); // sync real date / time on clock
  log(CLASS_MAIN, Info, "# Current time: %s", Timing::humanize(m.getBot()->getClock()->currentTime(), &timeAux));
  log(CLASS_MAIN, Info, "# Setup done.");
}

void configureMode() {
  unsigned long cycleBegin = millis();
  configureModeArchitecture();
  sleepInterruptable(cycleBegin, PERIOD_CONFIGURE_MSEC);
}

void runMode() {
  unsigned long cycleBegin = millis();
  log(CLASS_MAIN, Info, "BEGIN RUN MODE (ver: %s)\n\n", STRINGIFY(PROJ_VERSION));

  runModeArchitecture();

  m.loop(false, false, true);

#ifdef DEEP_SLEEP_MODE
  log(CLASS_MAIN, Warn, "Syncing actors with server (run)...");
  m.getPropSync()->serverSyncActors(); // sync properties from the server (with new props and new clock blocked timing)
#endif
  sleepInterruptable(cycleBegin, PERIOD_MSEC);
  log(CLASS_MAIN, Info, "END RUN MODE (ver: %s)\n\n", STRINGIFY(PROJ_VERSION));
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
