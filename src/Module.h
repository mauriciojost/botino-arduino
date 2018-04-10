#ifndef MODULE_INC
#define MODULE_INC

#include <main4ino/Actor.h>
#include <actors/Messenger.h>
#include <actors/Led.h>
#include <actors/Lcd.h>
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
  Led *led0;
  Led *led1;
  Settings *settings;
  WebBot *bot;
  Lcd *lcd;
  Body *body;

public:
  Module() {

    lcd = new Lcd("lc");
    msgr = new Messenger("m");
    led0 = new Led("l0", LED0_PIN);
    led1 = new Led("l1", LED1_PIN);
    clock = new Clock("c");
    settings = new Settings("g");
    body = new Body("b");

    clock->setFactor(PERIOD_SEC);

    actors = new Array<Actor *>(7);
    actors->set(0, (Actor *)clock);
    actors->set(1, (Actor *)msgr);
    actors->set(2, (Actor *)led0);
    actors->set(3, (Actor *)led1);
    actors->set(4, (Actor *)lcd);
    actors->set(5, (Actor *)settings);
    actors->set(6, (Actor *)body);

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

  void setDigitalWriteFunction(void (*digitalWriteFunction)(unsigned char pin, unsigned char value)) {
    led0->setDigitalWriteFunction(digitalWriteFunction);
    led1->setDigitalWriteFunction(digitalWriteFunction);
  }

  void setBotStdoutWriteFunction(void (*stdOutWriteStringFunction)(const char *, const char *)) {
    bot->setStdoutFunction(stdOutWriteStringFunction);
  }

  void setLcdStdoutWriteFunction(void (*stdOutWriteStringFunction)(int, const char *)) {
    lcd->setStdoutFunction(stdOutWriteStringFunction);
  }

  void setFactor(float f) {
    clock->setFactor(f);
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

};

#endif // MODULE_INC
