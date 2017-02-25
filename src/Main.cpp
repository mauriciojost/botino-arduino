#ifndef UNIT_TEST
#include <Main.h>

#define CLASS "Main"
#define TICKS_PERIOD_TIMER1 2500000

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

ICACHE_RAM_ATTR void timingInterrupt(void) {
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

void setupPins() {
  log(CLASS, Info, "PINS READY");
}

void setupInterrupts() {
  timer1_disable();
  timer1_isr_init();
  timer1_attachInterrupt(timingInterrupt);
  timer1_enable(TIM_DIV265, TIM_EDGE, TIM_LOOP);
  timer1_write(TICKS_PERIOD_TIMER1);
  log(CLASS, Info, "INT READY");
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
  bool wdtInterrupt = nroInterruptsQueued > 0;

  if (wdtInterrupt) {
    ButtonPressed button = readButtons();
    nroInterruptsQueued--;
    log(CLASS, Info, "INT");
    m.loop(button == ButtonModeWasPressed, button == ButtonSetWasPressed, wdtInterrupt);
    //m.loop(button == ButtonModeWasPressed, button == ButtonSetWasPressed, true);
    m.getClock()->setNroInterruptsQueued(nroInterruptsQueued);
  }

  if (nroInterruptsQueued <= 0) { // no interrupts queued
    nroInterruptsQueued = 0;
  }

}

#endif // UNIT_TEST
