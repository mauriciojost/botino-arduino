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

bool initWifiInit() {
  log(CLASS_MAIN, Info, "W.init");
  messageFuncExt(0, 2, "HOTSPOT?");
  delay(USER_DELAY_MS);
  messageFuncExt(0, 2, WIFI_SSID_INIT);
  delay(USER_DELAY_MS);
  bool connected = initWifi(WIFI_SSID_INIT, WIFI_PASSWORD_INIT, false, 20);
  if (connected) {
    messageFuncExt(0, 2, "HOTSPOT OK");
    delay(USER_DELAY_MS);
  } else {
    messageFuncExt(0, 2, "HOTSPOT KO");
    delay(USER_DELAY_MS);
  }
  return connected;
}

bool initWifiSteady() {
  SetupSync *s = m.getSetupSync();
  static bool connectedOnce = false;
  if (s->isInitialized()) {
    const char *wifiSsid = s->getSsid();
    const char *wifiPass = s->getPass();
    log(CLASS_MAIN, Info, "W.steady");
    bool connected = initWifi(wifiSsid, wifiPass, connectedOnce, 10);
    if (!connectedOnce) {
      messageFuncExt(0, 2, "WIFI?");
      delay(USER_DELAY_MS);
      messageFuncExt(0, 2, wifiSsid);
      delay(USER_DELAY_MS);
      if (connected) { // first time
        messageFuncExt(0, 2, "WIFI OK");
        delay(USER_DELAY_MS * 2);
      } else {
        messageFuncExt(0, 2, "WIFI KO");
      }
    }
    connectedOnce = connectedOnce || connected;
    return connected;
  } else {
    log(CLASS_MAIN, Info, "W.steady null");
    return false;
  }
}

void messageFuncExt(int line, int size, const char *format, ...) {
  Buffer<MAX_LOG_MSG_LENGTH - 1> buffer;
  va_list args;
  va_start(args, format);
  vsnprintf(buffer.getUnsafeBuffer(), MAX_LOG_MSG_LENGTH, format, args);
  buffer.getUnsafeBuffer()[MAX_LOG_MSG_LENGTH - 1] = 0;
  log(CLASS_MAIN, Debug, "MSG: %s", buffer.getBuffer());
  messageFunc(0, buffer.getBuffer(), size);
  va_end(args);
}



void setup() {

	m.setup(
    lcdImg,
    arms,
    messageFunc,
    ios,
    initWifiInit,
    initWifiSteady,
    httpPost,
    httpGet,
    clearDevice
  );

  setupArchitecture();
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
