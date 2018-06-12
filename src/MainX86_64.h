
#include <Main.h>

#define CL_MAX_LENGTH 1000
#define CURL_COMMAND_GET "curl --silent -XGET '%s'"
#define CURL_COMMAND_POST "curl --silent -XPOST '%s' -d '%s'"
#define HTTP_OK 200

bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries) {
  log(CLASS_MAIN, Debug, "initWifi(%s, %s, %d)", ssid, pass, retries);
  return true;
}

void handleSettings() {
  Settings *s = m.getSettings();
  // Handle log level as per settings
  setLogLevel((char)(s->getLogLevel() % 4));
}

void handleServices() {
  // noting to do here
}

void reactButton() {
  // noting to do here
}

/*****************/
/*** CALLBACKS ***/
/*****************/

void messageOnLcd(int line, const char *str, int size) {
  printf("LCD: %s (size %d)", str, size);
}

void logLine(const char *str) {
  printf("LOG: %s", str);
}

void arms(int left, int right, int steps) {
  printf("ARMS: %d %d %d", left, right, steps);
}

int httpGet(const char *url, ParamStream *response) {
  Buffer<CL_MAX_LENGTH> aux;
  aux.fill(CURL_COMMAND_GET, url);
  log(CLASS_MAIN, Debug, "GET: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {return -1;}
  while (fgets(aux.getUnsafeBuffer(), 1000 -1, fp) != NULL) {
    response->fill(aux.getBuffer());
  }
  log(CLASS_MAIN, Debug, "-> %s", response->content());
  pclose(fp);
  return HTTP_OK; // not quite true, but will work for simple purposes
}

int httpPost(const char *url, const char *body, ParamStream *response) {
  Buffer<CL_MAX_LENGTH> aux;
  aux.fill(CURL_COMMAND_POST, url, body);
  log(CLASS_MAIN, Debug, "POST: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {return -1;}
  while (fgets(aux.getUnsafeBuffer(), CL_MAX_LENGTH -1, fp) != NULL) {
    response->fill(aux.getBuffer());
  }
  log(CLASS_MAIN, Debug, "-> %s", response->content());
  pclose(fp);
  return HTTP_OK; // not quite true, but will work for simple purposes
}

void ios(char led, bool v) {
  log(CLASS_MAIN, Debug, "Led'%c'->%d", led, (int)v);
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "Img '%c'", img);
}

/*****************/
/***** SETUP *****/
/*****************/

void setup() {
  log(CLASS_MAIN, Debug, "Setup module");
  m.getBody()->setLcdImgFunc(lcdImg);
  m.getBody()->setArmsFunc(arms);
  m.getBody()->setMessageFunc(messageOnLcd);
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
}

void sleepInterruptable(unsigned long cycleBegin) {
  log(CLASS_MAIN, Info, "L.Sleep(%lums)...", PERIOD_MSEC);
  unsigned long spentMs = millis() - cycleBegin;
  log(CLASS_MAIN, Info, "D.C.:%0.3f", (float)spentMs / PERIOD_MSEC);
  while (spentMs < PERIOD_MSEC) {
    reactButton();
    unsigned long fragToSleepMs = MINIM(PERIOD_MSEC - spentMs, FRAG_TO_SLEEP_MS_MAX);
    delay(fragToSleepMs);
    spentMs = millis() - cycleBegin;
  }

}

int main( int argc, const char* argv[] ) {
	setup();
	while(true) {
    loop();
	}
}
