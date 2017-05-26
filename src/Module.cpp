#include <Module.h>

#define CLASS "Module"

Module::Module() {

  this->msgr = new Messenger("msgr0");
  this->led0 = new Led("led0");
  this->led1 = new Led("led1");
  this->clock = new Clock("clock0");

  actors = new Array<Actor*>(4);
  actors->set(0, (Actor*)clock);
  actors->set(1, (Actor*)msgr);
  actors->set(2, (Actor*)led0);
  actors->set(3, (Actor*)led1);

  this->bot = new WebBot(clock, actors);
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
    digitalWrite(LED0_PIN, ledValue.get());
    led1->getActuatorValue(&ledValue);
    digitalWrite(LED1_PIN, ledValue.get());
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

