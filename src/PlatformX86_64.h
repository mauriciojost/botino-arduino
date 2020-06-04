#include <Platform.h>
#include <cstdio>
#include <iostream>
#include <primitives/BoardX86_64.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifndef SIMULATOR_LOGIN
#error "Must define SIMULATOR_LOGIN"
#endif // SIMULATOR_LOGIN

#ifndef SIMULATOR_PASS
#error "Must define SIMULATOR_PASS"
#endif // SIMULATOR_PASS

#define CL_MAX_LENGTH 65000

enum AppMode { Interactive = 0, NonInteractive = 1 };
AppMode appMode = Interactive;

////////////////////////////////////////
// Functions requested for architecture
////////////////////////////////////////

// Callbacks
///////////////////

bool buttonIsPressed() {
  return false;
}

const char *apiDeviceLogin() {
  return SIMULATOR_LOGIN;
}

const char *apiDevicePass() {
  return SIMULATOR_PASS;
}

void logLine(const char *str, const char *clz, LogLevel l, bool newline) {}

void messageFunc(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str) {
  printf("\n\n***** LCD (size %d)\n  %s\n*****\n\n", size, str);
}

void arms(int left, int right, int steps) {
  printf("\n\nARMS: %d %d %d\n\n", left, right, steps);
}

void ios(char led, IoMode v) {
  log(CLASS_PLATFORM, Debug, "Led'%c'->%d", led, v);
}

void clearDevice() {
  log(CLASS_PLATFORM, Debug, "Clear device");
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_PLATFORM, Debug, "Img '%c'", img);
}

void infoArchitecture() {}

void testArchitecture() {}

// Execution
///////////////////

void heartbeat() {}

bool haveToInterrupt() {
  return false;
}

void setupArchitecture() {
  log(CLASS_PLATFORM, Debug, "Setup timing");
  setExternalMillis(millis);
  setupLog(logLine);
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
  log(CLASS_PLATFORM, Error, "Abort: %s", msg);
}

void askStringQuestion(const char *question, Buffer *answer) {}

////////////////////////////////////////
// Architecture specific functions
////////////////////////////////////////

void setup();
void loop();

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
    log(CLASS_PLATFORM, Error, "2 args max: <starter> [appMode [steps]]");
    return -1;
  }

  for (int i = 0; i < simulationSteps; i++) {
    log(CLASS_PLATFORM, Debug, "### Step %d/%d", i, simulationSteps);
    loop();
  }
  log(CLASS_PLATFORM, Debug, "### DONE");
  return 0;
}
