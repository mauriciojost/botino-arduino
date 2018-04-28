#ifndef MODULE_INC
#define MODULE_INC

#include <main4ino/Actor.h>
#include <actors/Messenger.h>
#include <actors/Led.h>
#include <actors/Arm.h>
#include <actors/Body.h>
#include <main4ino/Clock.h>
#include <main4ino/WebBot.h>
#include <main4ino/Array.h>
#include <log4ino/Log.h>
#include <Pinout.h>
#include "actors/Settings.h"

#define CLASS_MODULE "MD"

/**
* This class represents the integration of all components (LCD, buttons, buzzer, etc).
*/
class Module {

private:
  Messenger *msgr;
  Array<Actor *> *actors;
  Clock *clock;
  Settings *settings;
  WebBot *bot;
  Body *body;

public:
  Module() {

    msgr = new Messenger("m");
    clock = new Clock("c");
    settings = new Settings("g");
    body = new Body("b");

    actors = new Array<Actor *>(4);
    actors->set(0, (Actor *)msgr);
    actors->set(1, (Actor *)clock);
    actors->set(2, (Actor *)settings);
    actors->set(3, (Actor *)body);

    bot = new WebBot(clock, actors);
    msgr->setBot(bot);
    bot->setMode(RunMode);
  }

  void loop(bool mode, bool set, bool wdtWasTriggered) {

    bool anyButtonPressed = mode || set;
    TimingInterrupt interruptType = TimingInterruptNone;

    if (wdtWasTriggered) {
      interruptType = TimingInterruptCycle;
    }

    // execute a cycle on the bot
    bot->cycle(mode, set, interruptType);
  }

  void setup() {}

  Bot *getBot() {
  	return bot;
  }

  Clock *getClock() {
    return clock;
  }

  Settings *getSettings() {
    return settings;
  }

  Body *getBody() {
    return body;
  }

  Messenger *getMessenger() {
  	return msgr;
  }
};

#endif // MODULE_INC
