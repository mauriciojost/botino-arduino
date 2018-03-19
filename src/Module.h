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

#define GPIO0_PIN 0
#define GPIO1_PIN 1
#define GPIO2_PIN 2
#define GPIO3_PIN 3
#define GPIO4_PIN 4
#define GPIO5_PIN 5
#define GPIO6_PIN 6
#define GPIO7_PIN 7
#define GPIO8_PIN 8
#define GPIO9_PIN 9
#define GPIO10_PIN 10
#define GPIO11_PIN 11
#define GPIO12_PIN 12
#define GPIO13_PIN 13
#define GPIO14_PIN 14
#define GPIO15_PIN 15
#define GPIO16_PIN 16


#define PIN_D3 GPIO0_PIN
#define PIN_D10 GPIO1_PIN // (!) TX, if used will break serial communication
#define PIN_D4 GPIO2_PIN
#define PIN_D9 GPIO3_PIN
#define PIN_D2 GPIO4_PIN
#define PIN_D1 GPIO5_PIN
#define PIN_D11 GPIO9_PIN
#define PIN_D12 GPIO10_PIN
#define PIN_D6 GPIO12_PIN
#define PIN_D7 GPIO13_PIN
#define PIN_D5 GPIO14_PIN
#define PIN_D8 GPIO15_PIN
#define PIN_D0 GPIO16_PIN

#define LED0_PIN PIN_D0 // will break deep sleep mode
#define LED1_PIN PIN_D1 // will break deep sleep mode
#define LCD_BACKLIGHT_PIN PIN_D8
#define SERVO0_PIN PIN_D6 // using it breaks serial communication
#define LCD_ENABLE_PIN PIN_D2
#define LCD_RS_PIN PIN_D3

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
