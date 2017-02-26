#include <Module.h>

#define CLASS "Module"

Module::Module() {

#define NRO_ACTORS 3

  this->amountOfActors = NRO_ACTORS;
  this->amountOfConfigurables = NRO_ACTORS;

  this->msgr = new Messenger();
  this->led0 = new Led();
  this->led1 = new Led();
  this->actors = new Actor *[NRO_ACTORS + 1]{msgr, led0, led1, NULL};

  this->clock = new Clock(actors, amountOfActors);

  this->configurables = new Configurable *[NRO_ACTORS + 1]{msgr, led0, led1, NULL};

  this->bot = new Bot(clock, actors, configurables);
  this->msgr->setBot(bot);
  this->bot->setMode(RunMode);

  this->subCycle = 0;
}

void Module::loop(bool mode, bool set, bool wdtWasTriggered) {

  bool anyButtonPressed = mode || set;
  TimingInterrupt interruptType = TimingInterruptNone;

  if (wdtWasTriggered) {
    subCycle = (subCycle + 1) % SUB_CYCLES_PER_CYCLE;
    if (subCycle == 0) {
      interruptType = TimingInterruptCycle;
    } else {
      interruptType = TimingInterruptSubCycle;
    }
  }
  
  // execute a cycle on the bot
  bot->cycle(mode, set, interruptType, ((float)subCycle) / SUB_CYCLES_PER_CYCLE);
  
  if (interruptType == TimingInterruptCycle) { // cycles (~1 second)
    bool onceIn2Cycles = (bot->getClock()->getSeconds() % 2) == 0;
    if (isThereErrorLogged() && onceIn2Cycles) {
      bot->stdOutWriteString(MSG_ERROR, getErrorLogged());
    }
    digitalWrite(0, led0->getActuatorValue());
    digitalWrite(2, led1->getActuatorValue());
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


