#ifndef MODULE_INC
#define MODULE_INC

#include <main4ino/Actor.h>
#include <actors/Messenger.h>
#include <actors/Led.h>
#include <actors/Lcd.h>
#include <actors/Arm.h>
#include <actors/Global.h>
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
  Led *led2;
  Led *led3;
  Led *led4;
  Led *led5;
  Led *led6;
  Led *led7;
  Led *led8;
  Global *global0;
  WebBot *bot;
  Lcd *lcd;

public:

  Module() {

    lcd = new Lcd("lc");
    msgr = new Messenger("m");
    led0 = new Led("l0", LED0_PIN);
    led1 = new Led("l1", LED1_PIN);
    led2 = new Led("l2", LED2_PIN);
    led3 = new Led("l3", LED3_PIN);
    led4 = new Led("l4", LED4_PIN);
    led5 = new Led("l5", LED5_PIN);
    led6 = new Led("l6", LED6_PIN);
    led7 = new Led("l7", LED7_PIN);
    led8 = new Led("l8", LED8_PIN);
    clock = new Clock("c");
    arm0 = new Arm("a");
    global0 = new Global("g");

    actors = new Array<Actor*>(14);
    actors->set(0, (Actor*)clock);
    actors->set(1, (Actor*)msgr);
    actors->set(2, (Actor*)led0);
    actors->set(3, (Actor*)led1);
    actors->set(4, (Actor*)led2);
    actors->set(5, (Actor*)led3);
    actors->set(6, (Actor*)led4);
    actors->set(7, (Actor*)led5);
    actors->set(8, (Actor*)led6);
    actors->set(9, (Actor*)led7);
    actors->set(10, (Actor*)led8);
    actors->set(11, (Actor*)lcd);
    actors->set(12, (Actor*)arm0);
    actors->set(13, (Actor*)global0);

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

  void setup() { }

  void setDigitalWriteFunction(void (*digitalWriteFunction)(unsigned char pin, unsigned char value)) {
  	led0->setDigitalWriteFunction(digitalWriteFunction);
  	led1->setDigitalWriteFunction(digitalWriteFunction);
  	//led2->setDigitalWriteFunction(digitalWriteFunction);
  	//led3->setDigitalWriteFunction(digitalWriteFunction);
  	//led4->setDigitalWriteFunction(digitalWriteFunction);
  	//led5->setDigitalWriteFunction(digitalWriteFunction);
  	//led6->setDigitalWriteFunction(digitalWriteFunction);
  	//led7->setDigitalWriteFunction(digitalWriteFunction);
  	//led8->setDigitalWriteFunction(digitalWriteFunction);
  }

  void setBotStdoutWriteFunction(void (*stdOutWriteStringFunction)(const char *, const char *)) {
    bot->setStdoutFunction(stdOutWriteStringFunction);
  }

  void setLcdStdoutWriteFunction(void (*stdOutWriteStringFunction)(int, const char *)) {
    lcd->setStdoutFunction(stdOutWriteStringFunction);
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

  Global * getGlobal() {
    return global0;
  }

};

#endif // MODULE_INC
