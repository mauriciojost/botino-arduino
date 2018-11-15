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

  log(CLASS_MAIN, Info, "\n\n# Setup module...");
  m.setup(lcdImg, arms, messageFunc, ios, initWifiSteady, httpPost, httpGet, clearDevice, readFile, writeFile);
  log(CLASS_MAIN, Info, "# Setup architecture...");
  setupArchitecture();
  log(CLASS_MAIN, Info, "# Loading credentials stored in FS...");
  m.getPropSync()->fsLoadActorsProps(); // load stored properties (most importantly credentials)
  log(CLASS_MAIN, Info, "# Syncing actors with server...");
  bool serSyncd = m.getPropSync()->serverSyncRetry(); // sync properties from the server
  time_t leftTime = m.getBot()->getClock()->currentTime();

  log(CLASS_MAIN, Info, "# Previous actors' times: %s...", Timing::humanize(leftTime, &timeAux));
  m.getBot()->setActorsTime(leftTime);
  log(CLASS_MAIN, Info, "# Syncing clock...");
  bool clockSyncd = m.getClockSync()->syncClock(m.getSettings()->inDeepSleepMode()); // sync real date / time on clock, block if in deep sleep
  log(CLASS_MAIN, Info, "# Current time: %s", Timing::humanize(m.getBot()->getClock()->currentTime(), &timeAux));

  if (serSyncd && clockSyncd) {
    m.getBot()->setMode(RunMode);
  } else {
    abort("Setup failed");
  }
}

void configureMode() {
  time_t cycleBegin = now();
  configureModeArchitecture();
  sleepInterruptable(cycleBegin, PERIOD_CONFIGURE_MSEC / 1000);
}

void runMode() {
  time_t cycleBegin = now();
  runModeArchitecture();
  m.loop(false, false, true);
  if (m.getSettings()->inDeepSleepMode()) {
  	// before going to deep sleep store in the server the last status of all actors
    log(CLASS_MAIN, Info, "Syncing actors with server (run)...");
    m.getPropSync()->serverSyncRetry(); // sync properties from the server (with new props and new clock blocked timing)
  }
  sleepInterruptable(cycleBegin, PERIOD_MSEC / 1000);
}

void loop() {
  log(CLASS_MAIN, Info, "BEGIN LOOP (ver: %s)\n\n", STRINGIFY(PROJ_VERSION));
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
  log(CLASS_MAIN, Info, "END LOOP\n\n");
}
