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

#define LED0_PIN PIN_D0
#define LED1_PIN PIN_D1
#define LCD_BACKLIGHT_PIN PIN_D8
#define BUZZER0_PIN PIN_D10
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
  Led *buzzer0;
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
