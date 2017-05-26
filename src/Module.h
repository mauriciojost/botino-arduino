#ifndef MODULE_INC
#define MODULE_INC

#include <main4ino/Actor.h>
#include <actors/Messenger.h>
#include <actors/Led.h>
#include <actors/Lcd.h>
#include <main4ino/Clock.h>
#include <main4ino/WebBot.h>
#include <main4ino/Array.h>
#include <log4ino/Log.h>

#define PIN_D3 0
#define PIN_D10 1
#define PIN_D4 2
#define PIN_D9 3
#define PIN_D2 4
#define PIN_D1 5
#define PIN_D11 9
#define PIN_D12 10
#define PIN_D6 12
#define PIN_D7 13
#define PIN_D5 14
#define PIN_D8 15
#define PIN_D0 16

#define LED0_PIN PIN_D0
#define LED1_PIN PIN_D1
#define LCD_ENABLE_PIN PIN_D2
#define LCD_RS_PIN PIN_D3

#define LCD_D4_PIN PIN_D4
#define LCD_D5_PIN PIN_D5
#define LCD_D6_PIN PIN_D6
#define LCD_D7_PIN PIN_D7


/**
* This class represents the integration of all components (LCD, buttons, buzzer, etc).
*/
class Module {

private:
  Messenger *msgr;
  Array<Actor*> *actors;
  Clock *clock;
  Led *led0;
  Led *led1;
  WebBot *bot;
  Lcd *lcd;

  void (*digitalWrite)(unsigned char pin, unsigned char value);

public:
  Module();

  void loop(bool mode, bool set, bool wdt);

  void setup();
  void setDigitalWriteFunction(void (*digitalWriteFunction)(unsigned char pin, unsigned char value));
  void setStdoutWriteFunction(void (*stdOutWriteStringFunction)(const char *, const char *));
  void setFactor(float f);

  Bot *getBot();
  Clock *getClock();
  Lcd *getLcd();

};

#endif // MODULE_INC
