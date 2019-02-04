#ifndef MODULE_INC
#define MODULE_INC

#include "actors/Quotes.h"
#include "actors/Settings.h"
#include <Pinout.h>
#include <actors/Body.h>
#include <actors/Ifttt.h>
#include <actors/Images.h>
#include <actors/Notifier.h>
#include <actors/Moves.h>
#include <log4ino/Log.h>
#include <main4ino/ClockSync.h>
#include <main4ino/Actor.h>
#include <main4ino/Array.h>
#include <main4ino/Clock.h>
#include <main4ino/PropSync.h>
#include <main4ino/SerBot.h>
#include <main4ino/Table.h>

#define CLASS_MODULE "MD"

#define COMMAND_MAX_LENGTH 128

#define PERIOD_CONFIGURE_MSEC 4000

#define HELP_COMMAND_CLI                                                                                                                        \
  "\n  run             : go to run mode"                                                                                                        \
  "\n  conf            : go to conf mode"                                                                                                       \
  "\n  info            : show info about the device"                                                                                            \
  "\n  test            : test the architecture/hardware"                                                                                        \
  "\n  update          : update the firmware"                                                                                                   \
  "\n  wifi            : init steady wifi"                                                                                                      \
  "\n  get             : display actors properties"                                                                                             \
  "\n  get ...         : display actor <actor> properties"                                                                                      \
  "\n  set ...         : set an actor property (example: 'set body msg0 HELLO')"                                                                \
  "\n  move ...        : execute a move (example: 'move A00C55')"                                                                               \
  "\n  logl [...]      : get / change log level to <x> (0 is more verbose, to 3 least verbose)"                                                 \
  "\n  clear           : clear device (eeprom and crashes stacktrace)"                                                                          \
  "\n  actall          : all act"                                                                                                               \
  "\n  touchall        : mark actors as 'changed' to force synchronization with the server"                                                     \
  "\n  actone ...      : make actor <x> act"                                                                                                    \
  "\n  rnd             : execute random routine"                                                                                                \
  "\n  lcd ...         : write on display <x> <y> <color> <wrap> <clear> <size> <str>"                                                          \
  "\n  wifissid ...    : set wifi ssid"                                                                                                         \
  "\n  wifipass ...    : set wifi pass"                                                                                                         \
  "\n  ifttttoken ...  : set ifttt token"                                                                                                       \
  "\n  store           : save properties in eeprom (mainly for credentials)"                                                                    \
  "\n  load            : load properties in eeprom (mainly for credentials)"                                                                    \
  "\n  ack             : notification read"                                                                                                     \
  "\n  help            : show this help"                                                                                                        \
  "\n  (all messages are shown as info log level)"                                                                                              \
  "\n"

/**
 * This class represents the integration of all components (LCD, buttons, buzzer, etc).
 */
class Module {

private:
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
  Moves *moves;

  bool (*initWifiSteadyFunc)();
  void (*clearDeviceFunc)();
  void (*messageFunct)(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str);
  void (*sleepInterruptable)(time_t cycleBegin, time_t periodSec);
  void (*configureModeArchitecture)();
  void (*runModeArchitecture)();
  bool (*fileRead)(const char *fname, Buffer* content);
  bool (*fileWrite)(const char *fname, const char* content);
  void (*info)();
  void (*test)();
  void (*update)();

  bool sync() {
    log(CLASS_MODULE, Info, "# Loading credentials stored in FS...");
    getPropSync()->fsLoadActorsProps(); // load stored properties (most importantly credentials)
    log(CLASS_MODULE, Info, "# Syncing actors with server...");
    bool serSyncd = getPropSync()->serverSyncRetry(false); // sync properties from the server
    time_t leftTime = getBot()->getClock()->currentTime();

    Buffer timeAux(19);
    log(CLASS_MODULE, Info, "# Previous actors' times: %s...", Timing::humanize(leftTime, &timeAux));
    getBot()->setActorsTime(leftTime);
    log(CLASS_MODULE, Info, "# Syncing clock...");
    bool clockSyncd = getClockSync()->syncClock(getSettings()->inDeepSleepMode()); // sync real date / time on clock, block if in deep sleep
    log(CLASS_MODULE, Info, "# Current time: %s", Timing::humanize(getBot()->getClock()->currentTime(), &timeAux));

    return serSyncd && clockSyncd;
  }

public:
  Module(const char* devId) {

    propSync = new PropSync(devId, "propsync");
    clockSync = new ClockSync(devId, "clocksync");
    clock = new Clock("clock");
    settings = new Settings("settings");
    quotes = new Quotes("quotes");
    body = new Body("body");
    images = new Images("images");
    ifttt = new Ifttt("ifttt");
    notifier = new Notifier("notifier");
    moves = new Moves("moves");

    actors = new Array<Actor *>(10);
    actors->set(0, (Actor *)propSync);
    actors->set(1, (Actor *)clockSync);
    actors->set(2, (Actor *)clock);
    actors->set(3, (Actor *)settings);
    actors->set(4, (Actor *)quotes);
    actors->set(5, (Actor *)body);
    actors->set(6, (Actor *)images);
    actors->set(7, (Actor *)ifttt);
    actors->set(8, (Actor *)notifier);
    actors->set(9, (Actor *)moves);

    bot = new SerBot(clock, actors);

    body->setQuotes(quotes);
    body->setImages(images);
    body->setIfttt(ifttt);
    body->setNotifier(notifier);

    initWifiSteadyFunc = NULL;
    clearDeviceFunc = NULL;
    messageFunct = NULL;
    sleepInterruptable = NULL;
    configureModeArchitecture = NULL;
    runModeArchitecture = NULL;
    fileRead = NULL;
    fileWrite = NULL;
    info = NULL;
    test = NULL;
    update = NULL;
  }

  void setup(BotMode (*setupArchitecture)(),
  		       void (*lcdImg)(char img, uint8_t bitmap[]),
             void (*arms)(int left, int right, int steps),
             void (*messageFunc)(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str),
             void (*ios)(char led, IoMode v),
             bool (*initWifiSteady)(),
             int (*httpPost)(const char *url, const char *body, ParamStream *response, Table *headers),
             int (*httpGet)(const char *url, ParamStream *response, Table *headers),
             void (*clearDevice)(),
             bool (*fileReadFunc)(const char *fname, Buffer* content),
             bool (*fileWriteFunc)(const char *fname, const char* content),
             void (*abortFunc)(const char *msg),
             void (*sleepInterruptableFunc)(time_t cycleBegin, time_t periodSec),
             void (*configureModeArchitectureFunc)(),
             void (*runModeArchitectureFunc)(),
             void (*infoFunc)(),
             void (*updateFunc)(),
             void (*testFunc)()
						 ) {

    notifier->setMessageFunc(messageFunc);
    notifier->setLcdImgFunc(lcdImg);
    body->setArmsFunc(arms);
    body->setIosFunc(ios);

    propSync->setup(bot, initWifiSteady, httpGet, httpPost, fileReadFunc, fileWriteFunc);
    clockSync->setup(bot->getClock(), initWifiSteady, httpGet);
    quotes->setHttpGet(httpGet);
    quotes->setInitWifi(initWifiSteady);
    ifttt->setInitWifi(initWifiSteady);
    ifttt->setHttpPost(httpPost);

    initWifiSteadyFunc = initWifiSteady;
    clearDeviceFunc = clearDevice;
    messageFunct = messageFunc;
    sleepInterruptable = sleepInterruptableFunc;
    configureModeArchitecture = configureModeArchitectureFunc;
    runModeArchitecture = runModeArchitectureFunc;
    fileRead = fileReadFunc;
    fileWrite = fileWriteFunc;
    info = infoFunc;
    test = testFunc;
    update = updateFunc;

    BotMode mode = setupArchitecture(); // module objects initialized, architecture can be initialized now

    getBot()->setMode(mode);

    if (mode == RunMode) {
      bool syncSucc = sync();
      if (!syncSucc) {
        abortFunc("Setup failed");
      }
    }
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
      messageFunct(atoi(x), atoi(y), atoi(color), atoi(wrap), (MsgClearMode)atoi(clear), atoi(size), str);
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
      const char *actor = strtok(NULL, " ");
      getProps(actor);
      return false;
    } else if (strcmp("run", c) == 0) {
      log(CLASS_MODULE, Info, "-> Run mode");
      bot->setMode(RunMode);
      return true;
    } else if (strcmp("conf", c) == 0) {
      log(CLASS_MODULE, Info, "-> Configure mode");
      bot->setMode(ConfigureMode);
      return true;
    } else if (strcmp("info", c) == 0) {
      info();
      return false;
    } else if (strcmp("test", c) == 0) {
      test();
      return false;
    } else if (strcmp("update", c) == 0) {
      update();
      return false;
    } else if (strcmp("clear", c) == 0) {
      clearDeviceFunc();
      return false;
    } else if (strcmp("logl", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        char ll = getLogLevel();
        log(CLASS_MODULE, Info, "Log level: %d", ll);
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
      settings->setSsid(c);
      log(CLASS_MODULE, Info, "Wifi ssid: %s", settings->getSsid());
      return false;
    } else if (strcmp("wifipass", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULE, Info, "Argument needed:\n  wifipass <pass>");
        return false;
      }
      settings->setPass(c);
      log(CLASS_MODULE, Info, "Wifi pass: %s", settings->getPass());
      return false;
    } else if (strcmp("wifi", c) == 0) {
      initWifiSteadyFunc();
      return false;
    } else if (strcmp("ifttttoken", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULE, Info, "Argument needed:\n  ifttttoken <token>");
        return false;
      }
      ifttt->setKey(c);
      log(CLASS_MODULE, Info, "Ifttt token: %s", ifttt->getKey());
      return false;
    } else if (strcmp("actall", c) == 0) {
      actall();
      return false;
    } else if (strcmp("touchall", c) == 0) {
      touchall();
      return false;
    } else if (strcmp("actone", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULE, Info, "Argument needed:\n  actone <actorname>");
        return false;
      }
      actone(c);
      return false;
    } else if (strcmp("rnd", c) == 0) {
      int routine = (int)random(getSettings()->getNroRoutinesForButton());
      log(CLASS_MODULE, Debug, "Routine %d...", routine);
      getBody()->performMove(routine);
      return false;
    } else if (strcmp("store", c) == 0) {
      propSync->fsStoreActorsProps(); // store credentials
      log(CLASS_MODULE, Info, "Properties stored locally");
      return false;
    } else if (strcmp("load", c) == 0) {
      propSync->fsLoadActorsProps(); // load mainly credentials already set
      log(CLASS_MODULE, Info, "Properties loaded from local copy");
      return false;
    } else if (strcmp("ack", c) == 0) {
      notifier->notificationRead();
      body->performMove("Z.");
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

  void cycleBot(bool mode, bool set, bool cycle) {
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

  Moves *getMoves() {
    return moves;
  }

  Body *getBody() {
    return body;
  }

  Ifttt *getIfttt() {
    return ifttt;
  }

  ClockSync *getClockSync() {
    return clockSync;
  }

  PropSync *getPropSync() {
    return propSync;
  }

  Notifier *getNotifier() {
    return notifier;
  }

  /**
   * Make all actors act
   */
  void actall() {
    for (int i = 0; i < getBot()->getActors()->size(); i++) {
      Actor *a = getBot()->getActors()->get(i);
      log(CLASS_MODULE, Info, "One off: %s", a->getName());
      a->oneOff();
    }
  }

  /**
   * Touch all actors (to force them to be syncrhonized)
   */
  void touchall() {
    for (int i = 0; i < getBot()->getActors()->size(); i++) {
      Actor *a = getBot()->getActors()->get(i);
      Metadata *m = a->getMetadata();
      log(CLASS_MODULE, Info, "Touch: %s", a->getName());
      m->changed();
    }
  }

  /**
   * Make a given by-name-actor act
   */
  void actone(const char* actorName) {
    for (int i = 0; i < getBot()->getActors()->size(); i++) {
      Actor *a = getBot()->getActors()->get(i);
      if (strcmp(a->getName(), actorName) == 0) {
        log(CLASS_MODULE, Info, "One off: %s", a->getName());
        a->oneOff();
      }
    }
  }

  /**
   * Execute a command given an index
   *
   * Thought to be used via single button devices, so that
   * a button pressed can execute one of many available commands.
   */
  bool sequentialCommand(int index, bool dryRun) {
    switch (index) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4: {
        int ind = index - 0;
        const char *mvName = getMoves()->getMoveName(ind);
        getNotifier()->message(0, 2, "%s?", mvName);
        if (!dryRun) {
          command(getMoves()->getMoveValue(ind));
        }
      } break;
      case 5: {
        getNotifier()->message(0, 2, "All act?");
        if (!dryRun) {
          command("actall");
          command("run");
          getNotifier()->message(0, 1, "All act one-off");
        }
      } break;
      case 6: {
        getNotifier()->message(0, 2, "Config mode?");
        if (!dryRun) {
          command("conf");
          getNotifier()->message(0, 1, "In config mode");
        }
      } break;
      case 7: {
        getNotifier()->message(0, 2, "Run mode?");
        if (!dryRun) {
          command("run");
          getNotifier()->message(0, 1, "In run mode");
        }
      } break;
      case 8: {
        getNotifier()->message(0, 2, "Show info?");
        if (!dryRun) {
          command("info");
        }
      } break;
      default: {
        getNotifier()->message(0, 2, "Abort?");
        if (!dryRun) {
          command("move Z.");
        }
      } break;
    }
    return false;
  }


  void getProps(const char* actorN) {
      Buffer contentAuxBuffer(256);
      Array<Actor *> *actors = bot->getActors();
      for (int i = 0; i < actors->size(); i++) {
        Actor *actor = actors->get(i);
        if (actorN == NULL || strcmp(actor->getName(), actorN) == 0) {
          bot->getPropsJson(&contentAuxBuffer, i, EXCLUSIVE_FILTER_MODE, SENSITIVE_PROP_PREFIX);
          log(CLASS_MODULE, Info, "'%s' -> %s", actor->getName(), contentAuxBuffer.getBuffer());
        }
      }
  }

  void configureMode() {
    time_t cycleBegin = now();
    configureModeArchitecture();
    sleepInterruptable(cycleBegin, PERIOD_CONFIGURE_MSEC / 1000);
  }

  void runMode() {
    time_t cycleBegin = now();
    runModeArchitecture();
    cycleBot(false, false, true);
    if (getSettings()->inDeepSleepMode()) {
      // before going to deep sleep store in the server the last status of all actors
      log(CLASS_MODULE, Info, "Syncing actors with server (run)...");
      getPropSync()->serverSyncRetry(false); // sync properties from the server (with new props and new clock blocked timing)
    }
    sleepInterruptable(cycleBegin, getSettings()->periodMsec() / 1000);
  }

  void loop() {
    switch (getBot()->getMode()) {
      case (RunMode):
        log(CLASS_MODULE, Info, "BEGIN LOOP (ver: %s)\n\n", STRINGIFY(PROJ_VERSION));
        runMode();
        log(CLASS_MODULE, Info, "END LOOP\n\n");
        break;
      case (ConfigureMode):
        configureMode();
        break;
      default:
        break;
    }
  }

};


#endif // MODULE_INC
