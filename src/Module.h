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
  Led *buzzer0;
  WebBot *bot;
  Lcd *lcd;

  void (*digitalWrite)(unsigned char pin, unsigned char value);

public:

  Module() {

    this->lcd = new Lcd();

    this->msgr = new Messenger("msgr0");
    this->led0 = new Led("led0");
    this->led1 = new Led("led1");
    this->buzzer0 = new Led("buz0");
    this->clock = new Clock("clock0");
    this->arm0 = new Arm("arm0");

    actors = new Array<Actor*>(7);
    actors->set(0, (Actor*)clock);
    actors->set(1, (Actor*)msgr);
    actors->set(2, (Actor*)led0);
    actors->set(3, (Actor*)led1);
    actors->set(4, (Actor*)buzzer0);
    actors->set(5, (Actor*)lcd);
    actors->set(6, (Actor*)arm0);

    this->bot = new WebBot(clock, actors);
    this->msgr->setBot(bot);
    this->bot->setMode(RunMode);

    this->digitalWrite = NULL;

  }

  void loop(bool mode, bool set, bool wdtWasTriggered) {

    bool anyButtonPressed = mode || set;
    TimingInterrupt interruptType = TimingInterruptNone;

    if (wdtWasTriggered) {
      interruptType = TimingInterruptCycle;
    }

    // execute a cycle on the bot
    bot->cycle(mode, set, interruptType);

    if (interruptType == TimingInterruptCycle) { // cycles (~1 second)
      bool onceIn2Cycles = (bot->getClock()->getSeconds() % 2) == 0;
      Integer ledValue;
      led0->getActuatorValue(&ledValue);
      //digitalWrite(LED0_PIN, ledValue.get());
      led1->getActuatorValue(&ledValue);
      digitalWrite(LED1_PIN, ledValue.get());
      buzzer0->getActuatorValue(&ledValue);
      digitalWrite(SERVO0_PIN, ledValue.get());
      digitalWrite(LCD_BACKLIGHT_PIN, lcd->getLight()); // TODO move all pin related stuff to Main and make callbacks
    }
  }

  void setup() {
    lcd->initialize();
  }

  void setDigitalWriteFunction(void (*digitalWriteFunction)(unsigned char pin, unsigned char value)) {
    digitalWrite = digitalWriteFunction;
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
