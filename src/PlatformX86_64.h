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

#ifndef COMMANDS_FILENAME
#define COMMANDS_FILENAME "/tmp/commands.list"
#endif // COMMANDS_FILENAME

#define CL_MAX_LENGTH 65000

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

void logLine(const char *str, const char *clz, LogLevel l, bool newline) {
  printf("%s", str);
}

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
  setupLog(logLine);
  log(CLASS_PLATFORM, Debug, "Setup timing");
  setExternalMillis(millis);
}

void runModeArchitecture() {
}

CmdExecStatus commandArchitecture(Cmd* cmd) {
  return NotFound;
}

void configureModeArchitecture() { }

void abort(const char *msg) {
  log(CLASS_PLATFORM, Error, "Abort: %s", msg);
}

void askStringQuestion(const char *question, Buffer *answer) {}

