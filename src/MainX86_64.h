#include <Main.h>

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstdio>

#define CL_MAX_LENGTH 1000
#define CURL_COMMAND_GET "curl --silent -XGET '%s'"
#define CURL_COMMAND_POST "curl --silent -H 'Content-Type: application/json' -XPOST '%s' -d '%s'"


enum AppMode { Interactive = 0, NonInteractive = 1};
AppMode appMode = Interactive;

unsigned long millis();

////////////////////////////////////////
// Functions requested for architecture
////////////////////////////////////////

void logLine(const char *str) {
  printf("LOG: %s", str);
}

bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries) {
  log(CLASS_MAIN, Debug, "initWifi(%s, %s, %d)", ssid, pass, retries);
  return true;
}

int httpGet(const char *url, ParamStream *response, Table* headers) {
  Buffer aux(CL_MAX_LENGTH);
  aux.fill(CURL_COMMAND_GET, url);
  log(CLASS_MAIN, Debug, "GET: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {
    return HTTP_BAD_REQUEST;
  }
  while (fgets(aux.getUnsafeBuffer(), CL_MAX_LENGTH - 1, fp) != NULL) {
    if (response != NULL) {
      response->fill(aux.getBuffer());
      log(CLASS_MAIN, Debug, "-> %s", response->content());
    }
  }
  pclose(fp);
  return HTTP_OK; // not quite true, but will work for simple purposes
}

int httpPost(const char *url, const char *body, ParamStream *response, Table* headers) {
  Buffer aux(CL_MAX_LENGTH);
  aux.fill(CURL_COMMAND_POST, url, body);
  log(CLASS_MAIN, Debug, "POST: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {
    return HTTP_BAD_REQUEST;
  }
  while (fgets(aux.getUnsafeBuffer(), CL_MAX_LENGTH - 1, fp) != NULL) {
    if (response != NULL) {
      response->fill(aux.getBuffer());
      log(CLASS_MAIN, Debug, "-> %s", response->content());
    }
  }
  pclose(fp);
  return HTTP_OK; // not quite true, but will work for simple purposes
}

void messageFunc(int line, const char *str, int size) {
  printf("\n\n***** LCD (size %d)\n  %s\n*****\n\n", size, str);
}

void arms(int left, int right, int steps) {
  printf("\n\nARMS: %d %d %d\n\n", left, right, steps);
}

void ios(char led, bool v) {
  log(CLASS_MAIN, Debug, "Led'%c'->%d", led, (int)v);
}

void clearDevice() {
  log(CLASS_MAIN, Debug, "Clear device");
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "Img '%c'", img);
}

void sleepInterruptable(unsigned long cycleBegin, unsigned long periodMsec) {
  log(CLASS_MAIN, Info, "L.Sleep(%lums)...", periodMsec);
  sleep(1);
}

bool haveToInterrupt() {
  // noting to do here
  return false;
}

void setupArchitecture() {
  setExternalMillis(millis);
}

void runModeArchitecture() {
  if (appMode == Interactive) {
    char str[100];
    fgets(str, 100, stdin);
    printf("Parsing: '%s'\n", str);
    if (strlen(str) != 0) {
      m.command(str);
    }
  }
}

void configureModeArchitecture() {
  // nothing to be done here
}

////////////////////////////////////////
// Architecture specific functions
////////////////////////////////////////

int main(int argc, const char *argv[]) {
  appMode = (AppMode)atoi(argv[1]);
  int simulationSteps = atoi(argv[2]);

  setup();
  for (int i = 0; i < simulationSteps; i++) {
    log(CLASS_MAIN, Debug, "### Step %d/%d", i, simulationSteps);
    loop();
  }
}

unsigned long millis() {
  static unsigned long boot = -1;
  struct timespec tms;
  if (clock_gettime(CLOCK_REALTIME, &tms)) {
    log(CLASS_MAIN, Warn, "Couldn't get time");
    return -1;
  }
  unsigned long m = tms.tv_sec * 1000 + tms.tv_nsec / 1000000;
  if (boot == -1) {
    boot = m;
  }
  // log(CLASS_MAIN, Debug, "Millis: %lu", m);
  return m - boot;
}
