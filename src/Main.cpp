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
  messageFuncExt(0, 2, WIFI_PASSWORD_INIT);
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
  Buffer<MAX_LOG_MSG_LENGTH-1> buffer;
  va_list args;
  va_start(args, format);
  vsnprintf(buffer.getUnsafeBuffer(), MAX_LOG_MSG_LENGTH, format, args);
  buffer.getUnsafeBuffer()[MAX_LOG_MSG_LENGTH - 1] = 0;
  log(CLASS_MAIN, Debug, "MSG: %s", buffer.getBuffer());
  buffer.replace(' ', '\n');
  messageFunc(0, buffer.getBuffer(), size);
  va_end(args);
}


bool command(const char *cmd) {

  char buf[COMMAND_MAX_LENGTH];
  strncpy(buf, cmd, COMMAND_MAX_LENGTH);
  log(CLASS_MAIN, Info, "Command: '%s'", buf);

  if (strlen(buf) == 0) {
  	return false;
  }

  char *c = strtok(buf, " ");

  if (strcmp("move", c) == 0) {
    c = strtok(NULL, " ");
    if (c == NULL) {
      log(CLASS_MAIN, Info, "Argument needed:\n  move <move>");
      return false;
    }
    log(CLASS_MAIN, Info, "-> Move %s", c);
    m.getBody()->performMove(c);
    return false;
  } else if (strcmp("set", c) == 0) {
    const char *actor = strtok(NULL, " ");
    const char *prop = strtok(NULL, " ");
    const char *v = strtok(NULL, " ");
    if (actor == NULL || prop == NULL || v == NULL) {
      log(CLASS_MAIN, Info, "Arguments needed:\n  set <actor> <prop> <value>");
      return false;
    }
    log(CLASS_MAIN, Info, "-> Set %s.%s = %s", actor, prop, v);
    Buffer<64> value(v);
    m.getBot()->setProp(actor, prop, &value);
    return false;
  } else if (strcmp("get", c) == 0) {
    log(CLASS_MAIN, Info, "-> Get");
    Array<Actor *> *actors = m.getBot()->getActors();
    for (int i = 0; i < actors->size(); i++) {
      Actor *actor = actors->get(i);
      log(CLASS_MAIN, Info, " '%s'", actor->getName());
      for (int j = 0; j < actor->getNroProps(); j++) {
        Buffer<COMMAND_MAX_LENGTH> value;
        actor->getPropValue(j, &value);
        log(CLASS_MAIN, Info, "   '%s': '%s'", actor->getPropName(j), value.getBuffer());
      }
      log(CLASS_MAIN, Info, " ");
    }
    log(CLASS_MAIN, Info, " ");
    return false;
  } else if (strcmp("run", c) == 0) {
    log(CLASS_MAIN, Info, "-> Run mode");
    m.getBot()->setMode(RunMode);
    return true;
  } else if (strcmp("conf", c) == 0) {
    log(CLASS_MAIN, Info, "-> Configure mode");
    m.getBot()->setMode(ConfigureMode);
    return true;
  } else if (strcmp("wifi", c) == 0) {
    initWifiSteady();
    return false;
  } else if (strcmp("clear", c) == 0) {
    clearDevice();
    return false;
  } else if (strcmp("logl", c) == 0) {
    c = strtok(NULL, " ");
    if (c == NULL) {
      log(CLASS_MAIN, Info, "Argument needed:\n  logl <loglevel>");
      return false;
    }
    int ll = atoi(c);
    setLogLevel(ll);
    log(CLASS_MAIN, Info, "Log level: %d", ll);
    return false;
  } else if (strcmp("help", c) == 0) {
    logLine(HELP_COMMAND_CLI);
    return false;
  } else {
    logLine("What? (try: 'help')");
    return false;
  }
}


void setup() {

  m.getBody()->setLcdImgFunc(lcdImg);
  m.getBody()->setArmsFunc(arms);
  m.getBody()->setMessageFunc(messageFunc);
  m.getBody()->setIosFunc(ios);
  m.getPropSync()->setInitWifi(initWifiSteady);
  m.getPropSync()->setHttpPost(httpPost);
  m.getPropSync()->setHttpGet(httpGet);
  m.getClockSync()->setInitWifi(initWifiSteady);
  m.getClockSync()->setHttpGet(httpGet);
  m.getSetupSync()->setInitWifiSteady(initWifiSteady);
  m.getSetupSync()->setInitWifiInit(initWifiInit);
  m.getSetupSync()->setHttpGet(httpGet);
  m.getQuotes()->setHttpGet(httpGet);
  m.getQuotes()->setInitWifi(initWifiSteady);
  m.getIfttt()->setInitWifi(initWifiSteady);
  m.getIfttt()->setHttpPost(httpPost);

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
