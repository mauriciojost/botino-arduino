#ifndef MODULE_INC
#define MODULE_INC

#include "actors/Messages.h"
#include "actors/Quotes.h"
#include "actors/Settings.h"
#include <Pinout.h>
#include <actors/Body.h>
#include <actors/ClockSync.h>
#include <actors/Images.h>
#include <actors/PropSync.h>
#include <actors/SetupSync.h>
#include <actors/Ifttt.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Array.h>
#include <main4ino/Clock.h>
#include <main4ino/SerBot.h>

#define CLASS_MODULE "MD"

#define COMMAND_MAX_LENGTH 128

#define HELP_COMMAND_CLI \
    "\n  run    : go to run mode" \
    "\n  conf   : go to conf mode" \
    "\n  wifi   : init steady wifi" \
    "\n  get    : display actors properties" \
    "\n  set    : set an actor property (example: 'set body msg0 HELLO')" \
    "\n  move   : execute a move (example: 'move A00C55')" \
    "\n  logl   : change log level" \
    "\n  clear  : clear crashes stacktrace" \
    "\n  help   : show this help" \
    "\n  (all messages are shown as info log level)" \
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
  Messages *messages;
  Ifttt *ifttt;

  bool (*initWifiSteadyFunc)();
  void (*clearDeviceFunc)();

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
    messages = new Messages("messages");
    ifttt = new Ifttt("ifttt");

    actors = new Array<Actor *>(10);
    actors->set(0, (Actor *)setupSync);
    actors->set(1, (Actor *)propSync);
    actors->set(2, (Actor *)clockSync);
    actors->set(3, (Actor *)clock);
    actors->set(4, (Actor *)settings);
    actors->set(5, (Actor *)quotes);
    actors->set(6, (Actor *)body);
    actors->set(7, (Actor *)images);
    actors->set(8, (Actor *)messages);
    actors->set(9, (Actor *)ifttt);

    bot = new SerBot(clock, actors);

    propSync->setBot(bot);
    clockSync->setClock(bot->getClock());
    body->setQuotes(quotes);
    body->setImages(images);
    body->setMessages(messages);

    initWifiSteadyFunc = NULL;
    clearDeviceFunc = NULL;

  }

  void setup(
    void (*lcdImg)(char img, uint8_t bitmap[]),
    void (*arms)(int left, int right, int steps),
    void (*messageFunc)(int line, const char *str, int size),
    void (*ios)(char led, bool v),
    bool (*initWifiInit)(),
    bool (*initWifiSteady)(),
    int (*httpPost)(const char *url, const char *body, ParamStream *response),
    int (*httpGet)(const char *url, ParamStream *response),
    void (*clearDevice)()
  ) {

    body->setLcdImgFunc(lcdImg);
    body->setArmsFunc(arms);
    body->setMessageFunc(messageFunc);
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

    bot->setMode(RunMode);
  }

bool command(const char *cmd) {

  char buf[COMMAND_MAX_LENGTH];
  strncpy(buf, cmd, COMMAND_MAX_LENGTH);
  log(CLASS_MODULE, Info, "Command: '%s'", buf);

  if (strlen(buf) == 0) {
  	return false;
  }

  char *c = strtok(buf, " ");

  if (strcmp("move", c) == 0) {
    c = strtok(NULL, " ");
    if (c == NULL) {
      log(CLASS_MODULE, Info, "Argument needed:\n  move <move>");
      return false;
    }
    log(CLASS_MODULE, Info, "-> Move %s", c);
    body->performMove(c);
    return false;
  } else if (strcmp("set", c) == 0) {
    const char *actor = strtok(NULL, " ");
    const char *prop = strtok(NULL, " ");
    const char *v = strtok(NULL, " ");
    if (actor == NULL || prop == NULL || v == NULL) {
      log(CLASS_MODULE, Info, "Arguments needed:\n  set <actor> <prop> <value>");
      return false;
    }
    log(CLASS_MODULE, Info, "-> Set %s.%s = %s", actor, prop, v);
    Buffer<64> value(v);
    bot->setProp(actor, prop, &value);
    return false;
  } else if (strcmp("get", c) == 0) {
    log(CLASS_MODULE, Info, "-> Get");
    Array<Actor *> *actors = bot->getActors();
    for (int i = 0; i < actors->size(); i++) {
      Actor *actor = actors->get(i);
      log(CLASS_MODULE, Info, " '%s'", actor->getName());
      for (int j = 0; j < actor->getNroProps(); j++) {
        Buffer<COMMAND_MAX_LENGTH> value;
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
      log(CLASS_MODULE, Info, "Argument needed:\n  logl <loglevel>");
      return false;
    }
    int ll = atoi(c);
    setLogLevel(ll);
    log(CLASS_MODULE, Info, "Log level: %d", ll);
    return false;
  } else if (strcmp("help", c) == 0) {
    log(CLASS_MODULE, Warn, HELP_COMMAND_CLI);
    return false;
  } else {
    log(CLASS_MODULE, Warn, "What? (try: 'help', log level is Info)");
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

};

#endif // MODULE_INC
