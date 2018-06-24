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
  log(CLASS_MAIN, Info, "W.init %s", WIFI_SSID_INIT);
  messageFunc(0, "HOTSPOT?", 2);
  delay(1 * 2000);
  messageFunc(0, WIFI_SSID_INIT, 2);
  delay(1 * 2000);
  messageFunc(0, WIFI_PASSWORD_INIT, 2);
  delay(1 * 2000);
  bool connected = initWifi(WIFI_SSID_INIT, WIFI_PASSWORD_INIT, false, 20);
  if (connected) {
    messageFunc(0, "HOTSPOT OK", 2);
    delay(1 * 2000);
  } else {
    messageFunc(0, "HOTSPOT KO", 2);
    delay(1 * 2000);
  }
  return connected;
}

bool initWifiSteady() {
  SetupSync *s = m.getSetupSync();
  static bool connectedOnce = false;
  if (s->isInitialized()) {
    const char *wifiSsid = s->getSsid();
    const char *wifiPass = s->getPass();
    log(CLASS_MAIN, Info, "W.steady'%s'", wifiSsid);
    bool connected = initWifi(wifiSsid, wifiPass, connectedOnce, 10);
    if (!connectedOnce) {
      messageFunc(0, "WIFI?", 2);
      delay(1 * 2000);
      messageFunc(0, wifiSsid, 2);
      delay(1 * 2000);
      messageFunc(0, wifiPass, 2);
      delay(1 * 2000);
      if (connected) { // first time
        messageFunc(0, "WIFI OK", 2);
        log(CLASS_MAIN, Info, "WIFI OK");
        delay(10 * 1000);
      }
    }
    connectedOnce = connectedOnce || connected;
    return connected;
  } else {
    log(CLASS_MAIN, Info, "W.steady KO");
    return false;
  }
}

void reactCommand(const char *cmd) {
  char command[COMMAND_MAX_LENGTH];
  strncpy(command, cmd, COMMAND_MAX_LENGTH);
  log(CLASS_MAIN, Info, "Command: %s", command);

  char *c = strtok(command, " ");

  if (strcmp("conf", c) == 0) {
    log(CLASS_MAIN, Info, "-> Conf mode");
    m.getBot()->setMode(ConfigureMode);
    return;
  } else if (strcmp("move", c) == 0) {
    c = strtok(NULL, " ");
    if (c == NULL) {
      log(CLASS_MAIN, Error, "Argument needed:\n  move <move>");
      return;
    }
    log(CLASS_MAIN, Info, "-> Move %s", c);
    m.getBody()->performMove(c);
    return;
  } else if (strcmp("set", c) == 0) {
    const char *actor = strtok(NULL, " ");
    const char *prop = strtok(NULL, " ");
    const char *v = strtok(NULL, " ");
    if (actor == NULL || prop == NULL || v == NULL) {
      log(CLASS_MAIN, Error, "Arguments needed:\n  set <actor> <prop> <value>");
      return;
    }
    log(CLASS_MAIN, Info, "-> Set %s.%s = %s", actor, prop, v);
    Buffer<64> value(v);
    m.getBot()->setProp(actor, prop, &value);
    return;
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
    return;
  } else if (strcmp("run", c) == 0) {
    log(CLASS_MAIN, Info, "-> Run mode");
    m.getBot()->setMode(RunMode);
    return;
  } else {
    log(CLASS_MAIN, Error, "Invalid command (try: ?)");
    return;
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

  setupArchitecture();
}

void loop() {
  loopArchitecture();
  switch (m.getBot()->getMode()) {
    case (RunMode):
      m.loop(false, false, true);
      sleepInterruptable(PERIOD_MSEC);
      break;
    case (ConfigureMode):
      break;
    default:
      m.getBot()->setMode(RunMode);
      break;
  }
}
