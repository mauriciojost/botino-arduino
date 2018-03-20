#ifndef MODULE_INC
#define MODULE_INC

#include <main4ino/Actor.h>
#include <actors/Messenger.h>
#include <actors/Led.h>
#include <actors/Lcd.h>
#include <actors/Arm.h>
#include <main4ino/Clock.h>
#include <main4ino/WebBot.h>
#include <main4ino/Array.h>
#include <log4ino/Log.h>
#include <Pinout.h>


#define CLASS_MODULE "MD"

/**
* This class represents the integration of all components (LCD, buttons, buzzer, etc).
*/
class Module {

private:
  Messenger *msgr;
  Array<Actor*> *actors;
  Clock *clock;
  Arm *arm0;
  Led *led0;
  Led *led1;
  WebBot *bot;
  Lcd *lcd;

public:

  Module() {

    lcd = new Lcd();

    msgr = new Messenger("msgr0");
    led0 = new Led("led0", LED0_PIN);
    led1 = new Led("led1", LED1_PIN);
    clock = new Clock("clock0");
    arm0 = new Arm("arm0");

    actors = new Array<Actor*>(6);
    actors->set(0, (Actor*)clock);
    actors->set(1, (Actor*)msgr);
    actors->set(2, (Actor*)led0);
    actors->set(3, (Actor*)led1);
    actors->set(4, (Actor*)lcd);
    actors->set(5, (Actor*)arm0);

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

  void setup() {
    lcd->initialize();
  }

  void setDigitalWriteFunction(void (*digitalWriteFunction)(unsigned char pin, unsigned char value)) {
  	led0->setDigitalWriteFunction(digitalWriteFunction);
  	led1->setDigitalWriteFunction(digitalWriteFunction);
  }

  void setStdoutWriteFunction(void (*stdOutWriteStringFunction)(const char *, const char *)) {
    bot->setStdoutFunction(stdOutWriteStringFunction);
  }

  void setServoPositionFunction(void (*f)(int)) {
    arm0->setServoPositionFunction(f);
  }

  void setFactor(float f) {
    clock->setFactor(f);
  }

  Clock * getClock() {
    return clock;
  }

  Lcd * getLcd() {
    return lcd;
  }

};

#endif // MODULE_INC
