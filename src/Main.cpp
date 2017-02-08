#ifndef UNIT_TEST
#include <Main.h>

#include <Module.h>

#define CLASS "Main"

volatile char nroInterruptsQueued = 0; // counter to keep track of amount of timing
                                       // interrupts queued

enum ButtonPressed {
  NoButton = 0,
  ButtonSetWasPressed,
  ButtonModeWasPressed
};

Module m;

/*****************/
/** INTERRUPTS ***/
/*****************/

void timingInterrupt(void) {
  timer0_write(ESP.getCycleCount() + 41660000);
  nroInterruptsQueued++;
}

/******************/
/***  CALLBACKS ***/
/******************/

void displayOnLogs(const char *str1, const char *str2) {
  log(CLASS, Info, str1);
  log(CLASS, Info, str2);
}

/*****************/
/***** SETUP *****/
/*****************/

void setupPins() { }

void setupInterrupts() {
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(timingInterrupt);
  timer0_write(ESP.getCycleCount() + 41660000);
  interrupts();
}

void setup() {
  setupLog();
  delay(1000);
  log(CLASS, Info, "LOG READY");

  setupPins();
  m.setup();
  m.setStdoutWriteFunction(displayOnLogs);

  setupInterrupts();
}

ButtonPressed readButtons() {
  if (readAvailable() > 0) {
    int b = readByte();
    switch(b) {
      case 's':
      case 'S':
        return ButtonSetWasPressed;
      case 'm':
      case 'M':
        return ButtonModeWasPressed;
      default:
        return NoButton;
    }
  }
}

void loop() {
  ButtonPressed button = readButtons();
  bool wdtInterrupt = nroInterruptsQueued > 0;

  if (wdtInterrupt) {
    nroInterruptsQueued--;
    log(CLASS, Info, "INT");
    m.loop(button == ButtonModeWasPressed, button == ButtonSetWasPressed, wdtInterrupt);
    m.getClock()->setNroInterruptsQueued(nroInterruptsQueued);
  }

  if (nroInterruptsQueued <= 0) { // no interrupts queued
    nroInterruptsQueued = 0;
  }

  delay(1000);
}

#endif // UNIT_TEST
