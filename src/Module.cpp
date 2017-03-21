#include <Module.h>

#define CLASS "Module"

Module::Module() {

  this->msgr = new Messenger();
  this->led0 = new Led();
  this->led1 = new Led();
  actors.push_back((Actor*)&msgr);
  actors.push_back((Actor*)&led0);
  actors.push_back((Actor*)&led1);

  this->clock = new Clock();

  configurables.push_back((Actor*)&msgr);
  configurables.push_back((Actor*)&led0);
  configurables.push_back((Actor*)&led1);

  this->bot = new Bot(clock, actors, configurables);
  this->msgr->setBot(bot);
  this->bot->setMode(RunMode);
}

void Module::loop(bool mode, bool set, bool wdtWasTriggered) {

  bool anyButtonPressed = mode || set;
  TimingInterrupt interruptType = TimingInterruptNone;

  if (wdtWasTriggered) {
    interruptType = TimingInterruptCycle;
  }
  
  // execute a cycle on the bot
  bot->cycle(mode, set, interruptType);
  
  if (interruptType == TimingInterruptCycle) { // cycles (~1 second)
    bool onceIn2Cycles = (bot->getClock()->getSeconds() % 2) == 0;
    if (isThereErrorLogged() && onceIn2Cycles) {
      bot->stdOutWriteString(MSG_ERROR, getErrorLogged());
    }
    Integer ledValue;
    led0->getActuatorValue(&ledValue);
    digitalWrite(0, ledValue.get());
    led1->getActuatorValue(&ledValue);
    digitalWrite(2, ledValue.get());
  }

  if (anyButtonPressed) {
    clearErrorLogged();
  }
}

void Module::setup() {
}

void Module::setDigitalWriteFunction(void (*digitalWriteFunction)(unsigned char pin, unsigned char value)) {
  digitalWrite = digitalWriteFunction;
}

void Module::setStdoutWriteFunction(void (*stdOutWriteStringFunction)(const char *, const char *)) {
  bot->setStdoutFunction(stdOutWriteStringFunction);
}

void Module::setFactor(float f) {
  clock->setFactor(f);
}

Clock * Module::getClock() {
  return clock;
}

