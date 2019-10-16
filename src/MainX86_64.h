#include <Main.h>

#include <cstdio>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifndef SIMULATOR_LOGIN
#define SIMULATOR_LOGIN "simulator_login"
#endif // SIMULATOR_LOGIN

#ifndef SIMULATOR_PASS
#define SIMULATOR_PASS "simulator_pass"
#endif // SIMULATOR_PASS

#define CL_MAX_LENGTH 65000
#define HTTP_CODE_KEY "HTTP_CODE:"
#define CURL_COMMAND_GET "/usr/bin/curl --silent -w '" HTTP_CODE_KEY "%%{http_code}' -XGET '%s'"
#define CURL_COMMAND_POST "/usr/bin/curl --silent -w '" HTTP_CODE_KEY "%%{http_code}' -XPOST '%s' -d '%s'"

enum AppMode { Interactive = 0, NonInteractive = 1 };
AppMode appMode = Interactive;

#include <primitives/BoardX86_64.h>

////////////////////////////////////////
// Functions requested for architecture
////////////////////////////////////////

// Callbacks
///////////////////

bool initWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfAlreadyConnected, int retries) {
  return initializeWifi(ssid, pass, ssidb, passb, skipIfAlreadyConnected, retries);
}

const char *apiDeviceLogin() {
  return SIMULATOR_LOGIN;
}

const char *apiDevicePass() {
  return SIMULATOR_PASS;
}

void logLine(const char *str) {
  printf("LOG: %s", str);
}


void messageFunc(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str) {
  printf("\n\n***** LCD (size %d)\n  %s\n*****\n\n", size, str);
}

void arms(int left, int right, int steps) {
  printf("\n\nARMS: %d %d %d\n\n", left, right, steps);
}

void ios(char led, IoMode v) {
  log(CLASS_MAIN, Debug, "Led'%c'->%d", led, v);
}

void clearDevice() {
  log(CLASS_MAIN, Debug, "Clear device");
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "Img '%c'", img);
}


void infoArchitecture() {}

void testArchitecture() {}

void updateFirmwareVersion(const char *version) {}

// Execution
///////////////////

void heartbeat() { }

bool haveToInterrupt() {
  return false;
}

bool sleepInterruptable(time_t cycleBegin, time_t periodSecs) {
  return lightSleepInterruptable(cycleBegin, periodSecs, m->getModuleSettings()->miniPeriodMsec(), haveToInterrupt, heartbeat);
}

BotMode setupArchitecture() {
  log(CLASS_MAIN, Debug, "Setup timing");
  setExternalMillis(millis);
  return RunMode;
}

void runModeArchitecture() {
  if (appMode == Interactive) {
    char str[100];
    printf("Waiting for input: \n   ");
    fgets(str, 100, stdin);
    if (strlen(str) != 0) {
      Buffer cmdBuffer(str);
      cmdBuffer.replace('\n', 0);
      cmdBuffer.replace('\r', 0);
      m->command(cmdBuffer.getBuffer());
    }
  }
}

CmdExecStatus commandArchitecture(const char *c) {
  return NotFound;
}

void configureModeArchitecture() {
  // nothing to be done here
}

void abort(const char *msg) {
  log(CLASS_MAIN, Error, "Abort: %s", msg);
}

////////////////////////////////////////
// Architecture specific functions
////////////////////////////////////////

int main(int argc, const char *argv[]) {
  setup();

  int simulationSteps = 10;

  if (argc == 1) {
    appMode = NonInteractive;
  } else if (argc == 1 + 1) {
    appMode = (AppMode)atoi(argv[1]);
  } else if (argc == 1 + 2) {
    appMode = (AppMode)atoi(argv[1]);
    simulationSteps = atoi(argv[2]);
  } else if (argc != 1 + 2) {
    log(CLASS_MAIN, Error, "2 args max: <starter> [appMode [steps]]");
    return -1;
  }

  for (int i = 0; i < simulationSteps; i++) {
    log(CLASS_MAIN, Debug, "### Step %d/%d", i, simulationSteps);
    loop();
  }
  log(CLASS_MAIN, Debug, "### DONE");
  return 0;
}

