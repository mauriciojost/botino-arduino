#ifndef MODULE_INC
#define MODULE_INC

#include <main4ino/Actor.h>
#include <actors/sync/ClockSync.h>
#include <actors/sync/PropSync.h>
#include <actors/sync/SetupSync.h>
#include <actors/Body.h>
#include <main4ino/Clock.h>
#include <main4ino/SerBot.h>
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
  SetupSync *setupSync;
  PropSync *propSync;
  ClockSync *clockSync;
  Array<Actor *> *actors;
  Clock *clock;
  Settings *settings;
  SerBot *bot;
  Body *body;

public:
  Module() {

    setupSync = new SetupSync("setupsync");
    propSync = new PropSync("propsync");
    clockSync = new ClockSync("clocksync");
    clock = new Clock("clock");
    settings = new Settings("settings");
    body = new Body("body");

    actors = new Array<Actor *>(6);
    actors->set(0, (Actor *)setupSync);
    actors->set(1, (Actor *)propSync);
    actors->set(2, (Actor *)clockSync);
    actors->set(3, (Actor *)clock);
    actors->set(4, (Actor *)settings);
    actors->set(5, (Actor *)body);

    bot = new SerBot(clock, actors);

    propSync->setBot(bot);
    clockSync->setClock(bot->getClock());

    bot->setMode(RunMode);
  }

  void loop(bool mode, bool set, bool cycle) {

    bool anyButtonPressed = mode || set;
    TimingInterrupt interruptType = TimingInterruptNone;

    if (cycle) {
      interruptType = TimingInterruptCycle;
    }

    // execute a cycle on the bot
    bot->cycle(mode, set, interruptType);
  }

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

  SetupSync *getSetupSync() {
    return setupSync;
  }

  PropSync *getPropSync() {
    return propSync;
  }

  ClockSync *getClockSync() {
    return clockSync;
  }
};

#endif // MODULE_INC
