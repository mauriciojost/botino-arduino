#ifndef MODULE_INC
#define MODULE_INC

#include <main4ino/Actor.h>
#include <main4ino/Configurable.h>
#include <actors/Messenger.h>
#include <actors/Led.h>
#include <main4ino/Clock.h>
#include <main4ino/Bot.h>
#include <main4ino/Array.h>
#include <log4ino/Log.h>

/**
* This class represents the integration of all components (LCD, buttons, buzzer, etc).
*/
class Module {

private:
  Messenger *msgr;
  Array<Actor*> actors;
  Clock *clock;
  Led *led0;
  Led *led1;
  Array<Configurable*> configurables;
  Bot *bot;

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

};

#endif // MODULE_INC
