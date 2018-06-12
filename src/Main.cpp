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
  return initWifi(WIFI_SSID_INIT, WIFI_PASSWORD_INIT, false, 20);
}

bool initWifiSteady() {
  SetupSync *s = m.getSetupSync();
  static bool connectedOnce = false;
  if (s->isInitialized()) {
    const char *wifiSsid = s->getSsid();
    const char *wifiPass = s->getPass();
    log(CLASS_MAIN, Info, "W.steady %s", wifiSsid);
    bool connected = initWifi(wifiSsid, wifiPass, connectedOnce, 5);
    if (!connectedOnce) {
      messageFunc(0, "WIFI SETUP...", 2);
      delay(1 * 2000);
      messageFunc(0, wifiSsid, 2);
      delay(1 * 2000);
      if (connected) { // first time
        messageFunc(0, "SETUP OK", 2);
        log(CLASS_MAIN, Info, "SETUP OK");
        delay(10 * 1000);
      }
    }
    connectedOnce = connectedOnce || connected;
    return connected;
  } else {
    log(CLASS_MAIN, Info, "W.steady not ready");
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

  setupArchitecture();

}

void loop() {
  unsigned long t1 = millis();
  loopArchitecture();
  switch (m.getBot()->getMode()) {
    case (RunMode):
      m.loop(false, false, true);
      sleepInterruptable(t1);
      break;
    case (ConfigureMode):
      break;
    default:
      m.getBot()->setMode(RunMode);
      break;
  }
}
