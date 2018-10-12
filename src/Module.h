#ifndef MODULE_INC
#define MODULE_INC

#include "actors/Quotes.h"
#include "actors/Settings.h"
#include <Pinout.h>
#include <actors/Body.h>
#include <actors/ClockSync.h>
#include <actors/Ifttt.h>
#include <actors/Images.h>
#include <actors/Notifier.h>
#include <actors/SetupSync.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Array.h>
#include <main4ino/Clock.h>
#include <main4ino/PropSync.h>
#include <main4ino/SerBot.h>
#include <main4ino/Table.h>

#define CLASS_MODULE "MD"

#define COMMAND_MAX_LENGTH 128

#define HELP_COMMAND_CLI                                                                                                                   \
  "\n  run        : go to run mode"                                                                                                        \
  "\n  conf       : go to conf mode"                                                                                                       \
  "\n  wifi       : init steady wifi"                                                                                                      \
  "\n  get        : display actors properties"                                                                                             \
  "\n  set        : set an actor property (example: 'set body msg0 HELLO')"                                                                \
  "\n  move       : execute a move (example: 'move A00C55')"                                                                               \
  "\n  logl       : change log level"                                                                                                      \
  "\n  clear      : clear crashes stacktrace"                                                                                              \
  "\n  actall     : all act"                                                                                                               \
  "\n  rnd        : execute random routine"                                                                                                \
  "\n  lcd        : write on display <x> <y> <color> <wrap> <clear> <size> <str>"                                                          \
  "\n  wifissid   : set wifi ssid"                                                                                                         \
  "\n  wifipass   : set wifi pass"                                                                                                         \
  "\n  ifttttoken : set ifttt token"                                                                                                       \
  "\n  timezonekey: set timezonedb.com/v2 api key"                                                                                         \
  "\n  ack        : notification read"                                                                                                     \
  "\n  help       : show this help"                                                                                                        \
  "\n  (all messages are shown as info log level)"                                                                                         \
  "\n"

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
  Quotes *quotes;
  Images *images;
  Ifttt *ifttt;
  Notifier *notifier;

  bool (*initWifiSteadyFunc)();
  void (*clearDeviceFunc)();
  void (*messageFunct)(int x, int y, int color, bool wrap, bool clear, int size, const char *str);

public:
  Module() {

    setupSync = new SetupSync("setupsync");
    propSync = new PropSync("propsync");
    clockSync = new ClockSync("clocksync");
    clock = new Clock("clock");
    settings = new Settings("settings");
    quotes = new Quotes("quotes");
    body = new Body("body");
    images = new Images("images");
    ifttt = new Ifttt("ifttt");
    notifier = new Notifier("notifier");

    actors = new Array<Actor *>(10);
    actors->set(0, (Actor *)setupSync);
    actors->set(1, (Actor *)propSync);
    actors->set(2, (Actor *)clockSync);
    actors->set(3, (Actor *)clock);
    actors->set(4, (Actor *)settings);
    actors->set(5, (Actor *)quotes);
    actors->set(6, (Actor *)body);
    actors->set(7, (Actor *)images);
    actors->set(8, (Actor *)ifttt);
    actors->set(9, (Actor *)notifier);

    bot = new SerBot(clock, actors);

    propSync->setBot(bot);
    clockSync->setClock(bot->getClock());
    body->setQuotes(quotes);
    body->setImages(images);
    body->setIfttt(ifttt);
    body->setNotifier(notifier);

    initWifiSteadyFunc = NULL;
    clearDeviceFunc = NULL;
    messageFunct = NULL;
  }

  void setup(void (*lcdImg)(char img, uint8_t bitmap[]),
             void (*arms)(int left, int right, int steps),
             void (*messageFunc)(int x, int y, int color, bool wrap, bool clear, int size, const char *str),
             void (*ios)(char led, bool v),
             bool (*initWifiInit)(),
             bool (*initWifiSteady)(),
             int (*httpPost)(const char *url, const char *body, ParamStream *response, Table *headers),
             int (*httpGet)(const char *url, ParamStream *response, Table *headers),
             void (*clearDevice)()) {

    notifier->setMessageFunc(messageFunc);
    body->setLcdImgFunc(lcdImg);
    body->setArmsFunc(arms);
    body->setIosFunc(ios);
    propSync->setInitWifi(initWifiSteady);
    propSync->setHttpPost(httpPost);
    propSync->setHttpGet(httpGet);
    clockSync->setInitWifi(initWifiSteady);
    clockSync->setHttpGet(httpGet);
    quotes->setHttpGet(httpGet);
    quotes->setInitWifi(initWifiSteady);
    ifttt->setInitWifi(initWifiSteady);
    ifttt->setHttpPost(httpPost);

    initWifiSteadyFunc = initWifiSteady;
    clearDeviceFunc = clearDevice;
    messageFunct = messageFunc;

    bot->setMode(RunMode);
  }

  bool command(const char *cmd) {

    Buffer *b = new Buffer(COMMAND_MAX_LENGTH);

    b->load(cmd);
    b->replace('\n', 0);
    log(CLASS_MODULE, Info, "Command: '%s'", b->getBuffer());

    if (b->getLength() == 0) {
      return false;
    }

    char *c = strtok(b->getUnsafeBuffer(), " ");

    if (strcmp("move", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULE, Info, "Argument needed:\n  move <move>");
        return false;
      }
      log(CLASS_MODULE, Info, "-> Move %s", c);
      body->performMove(c);
      return false;
    } else if (strcmp("lcd", c) == 0) {

	  // int x, int y, int color, bool wrap, bool clear, int size, const char *str);
      const char *x = strtok(NULL, " ");
      const char *y = strtok(NULL, " ");
      const char *color = strtok(NULL, " ");
      const char *wrap = strtok(NULL, " ");
      const char *clear = strtok(NULL, " ");
      const char *size = strtok(NULL, " ");
      const char *str = strtok(NULL, " ");
      if (x == NULL || y == NULL || color == NULL || wrap == NULL || clear == NULL || size == NULL || str == NULL) {
        logRaw(CLASS_MODULE, Info, "Arguments needed:\n  lcd <x> <y> <color> <wrap> <clear> <size> <str>");
        return false;
      }
      log(CLASS_MODULE, Info, "-> Lcd %s", str);
      messageFunct(atoi(x), atoi(y), atoi(color), atoi(wrap), atoi(clear), atoi(size), str);
      return false;
    } else if (strcmp("set", c) == 0) {
      const char *actor = strtok(NULL, " ");
      const char *prop = strtok(NULL, " ");
      const char *v = strtok(NULL, " ");
      if (actor == NULL || prop == NULL || v == NULL) {
        logRaw(CLASS_MODULE, Info, "Arguments needed:\n  set <actor> <prop> <value>");
        return false;
      }
      log(CLASS_MODULE, Info, "-> Set %s.%s = %s", actor, prop, v);
      Buffer value(64, v);
      bot->setProp(actor, prop, &value);
      return false;
    } else if (strcmp("get", c) == 0) {
      log(CLASS_MODULE, Info, "-> Get");
      Array<Actor *> *actors = bot->getActors();
      for (int i = 0; i < actors->size(); i++) {
        Actor *actor = actors->get(i);
        log(CLASS_MODULE, Info, " '%s'", actor->getName());
        for (int j = 0; j < actor->getNroProps(); j++) {
          Buffer value(COMMAND_MAX_LENGTH);
          actor->getPropValue(j, &value);
          log(CLASS_MODULE, Info, "   '%s': '%s'", actor->getPropName(j), value.getBuffer());
        }
        log(CLASS_MODULE, Info, " ");
      }
      log(CLASS_MODULE, Info, " ");
      return false;
    } else if (strcmp("run", c) == 0) {
      log(CLASS_MODULE, Info, "-> Run mode");
      bot->setMode(RunMode);
      return true;
    } else if (strcmp("conf", c) == 0) {
      log(CLASS_MODULE, Info, "-> Configure mode");
      bot->setMode(ConfigureMode);
      return true;
    } else if (strcmp("wifi", c) == 0) {
      initWifiSteadyFunc();
      return false;
    } else if (strcmp("clear", c) == 0) {
      clearDeviceFunc();
      return false;
    } else if (strcmp("logl", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULE, Info, "Argument needed:\n  logl <loglevel>");
        return false;
      }
      int ll = atoi(c);
      setLogLevel(ll);
      log(CLASS_MODULE, Info, "Log level: %d", ll);
      return false;
    } else if (strcmp("wifissid", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULE, Info, "Argument needed:\n  wifissid <ssid>");
        return false;
      }
      setupSync->setSsid(c);
      log(CLASS_MODULE, Info, "Wifi ssid: %s", setupSync->getSsid());
      return false;
    } else if (strcmp("wifipass", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULE, Info, "Argument needed:\n  wifipass <pass>");
        return false;
      }
      setupSync->setPass(c);
      log(CLASS_MODULE, Info, "Wifi pass: %s", setupSync->getPass());
      return false;
    } else if (strcmp("ifttttoken", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULE, Info, "Argument needed:\n  ifttttoken <token>");
        return false;
      }
      setupSync->setIfttt(c);
      log(CLASS_MODULE, Info, "Ifttt token: %s", setupSync->getIfttt());
      return false;
    } else if (strcmp("actall", c) == 0) {
      for (int i = 0; i < getBot()->getActors()->size(); i++) {
        Actor *a = getBot()->getActors()->get(i);
        log(CLASS_MODULE, Info, "One off: %s", a->getName());
        a->oneOff();
      }
      return false;
    } else if (strcmp("rnd", c) == 0) {
      int routine = (int)random(getSettings()->getNroRoutinesForButton());
      log(CLASS_MODULE, Debug, "Routine %d...", routine);
      getBody()->performMove(routine);
      return false;
    } else if (strcmp("timezonekey", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULE, Info, "Argument needed:\n  timezonekey <key>");
        return false;
      }
      clockSync->setDbKey(c);
      log(CLASS_MODULE, Info, "TimeZoneDb key: %s", clockSync->getDbKey());
      return false;
    } else if (strcmp("ack", c) == 0) {
      c = strtok(NULL, " ");
      notifier->notificationRead();
      log(CLASS_MODULE, Info, "Notification read");
      return false;
    } else if (strcmp("help", c) == 0) {
      logRaw(CLASS_MODULE, Warn, HELP_COMMAND_CLI);
      return false;
    } else {
      logRaw(CLASS_MODULE, Warn, "What? (try: 'help', log level is Info)");
      return false;
    }
  }

  void loop(bool mode, bool set, bool cycle) {

    TimingInterrupt interruptType = TimingInterruptNone;

    if (cycle) {
      interruptType = TimingInterruptCycle;
    }

    // execute a cycle on the bot
    bot->cycle(mode, set, interruptType);
  }

  // All getters should be removed, and initialization of these instances below should
  // be done in Module itself. This should help decrease the size of
  Bot *getBot() {
    return bot;
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

  Ifttt *getIfttt() {
    return ifttt;
  }

  ClockSync *getClockSync() {
    return clockSync;
  }

  Notifier *getNotifier() {
    return notifier;
  }
};

#endif // MODULE_INC
