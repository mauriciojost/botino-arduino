#ifndef MODULE_BOTINO_INC
#define MODULE_BOTINO_INC

#include <Pinout.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>

#include <mod4ino/Module.h>
#include <actors/Body.h>
#include <actors/Commands.h>
#include <actors/Ifttt.h>
#include <actors/Images.h>
#include <actors/Notifier.h>
#include <actors/Quotes.h>

#define CLASS_MODULEB "MB"

#define COMMAND_MAX_LENGTH 128

#define PERIOD_CONFIGURE_MSEC 4000

#define HELP_COMMAND_CLI_PROJECT                                                                                                           \
  "\n  BOTINO HELP"                                                                                                                        \
  "\n  move ...        : execute a move (example: 'move A00C55')"                                                                          \
  "\n  lcd ...         : write on display <x> <y> <color> <wrap> <clear> <size> <str>"                                                     \
  "\n  ifttttoken ...  : set ifttt token"                                                                                                  \
  "\n  ack             : notification read"                                                                                                \
  "\n  help            : show this help"                                                                                                   \
  "\n"

/**
 * This class represents the integration of all components (LCD, buttons, buzzer, etc).
 */
class ModuleBotino {

private:
  Module *module;

  Body *body;
  Quotes *quotes;
  Images *images;
  Ifttt *ifttt;
  Notifier *notifier;
  Commands *commands;

  void (*message)(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str);

public:
  ModuleBotino() {

    module = new Module();

    quotes = new Quotes("quotes");
    body = new Body("body");
    images = new Images("images");
    ifttt = new Ifttt("ifttt");
    notifier = new Notifier("notifier");
    commands = new Commands("commands");

    module->getActors()->add(6, (Actor *)quotes, (Actor *)body, (Actor *)images, (Actor *)ifttt, (Actor *)notifier, (Actor *)commands);

    message = NULL;

  }

  void setup(BotMode (*setupArchitectureFunc)(),
             void (*lcdImgFunc)(char img, uint8_t bitmap[]),
             void (*armsFunc)(int left, int right, int steps),
             void (*messageFunc)(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str),
             void (*iosFunc)(char led, IoMode v),
             bool (*initWifiFunc)(),
             int (*httpPostFunc)(const char *url, const char *body, ParamStream *response, Table *headers),
             int (*httpGetFunc)(const char *url, ParamStream *response, Table *headers),
             void (*clearDeviceFunc)(),
             bool (*fileReadFunc)(const char *fname, Buffer *content),
             bool (*fileWriteFunc)(const char *fname, const char *content),
             void (*abortFunc)(const char *msg),
             bool (*sleepInterruptableFunc)(time_t cycleBegin, time_t periodSec),
             void (*configureModeArchitectureFunc)(),
             void (*runModeArchitectureFunc)(),
             bool (*commandArchitectureFunc)(const char *cmd),
             void (*infoFunc)(),
             void (*updateFunc)(const char*),
             void (*testFunc)(),
             const char *(*apiDeviceLoginFunc)(),
             const char *(*apiDevicePassFunc)()) {

    module->setup(setupArchitectureFunc,
                  initWifiFunc,
                  httpPostFunc,
                  httpGetFunc,
                  clearDeviceFunc,
                  fileReadFunc,
                  fileWriteFunc,
                  abortFunc,
                  sleepInterruptableFunc,
                  configureModeArchitectureFunc,
                  runModeArchitectureFunc,
                  commandArchitectureFunc,
                  infoFunc,
                  updateFunc,
                  testFunc,
                  apiDeviceLoginFunc,
                  apiDevicePassFunc);

    message = messageFunc;

    body->setQuotes(quotes);
    body->setImages(images);
    body->setIfttt(ifttt);
    body->setNotifier(notifier);

    notifier->setup(lcdImgFunc, messageFunc);
    quotes->setHttpGet(httpGetFunc);
    quotes->setInitWifi(initWifiFunc);
    ifttt->setInitWifi(initWifiFunc);
    ifttt->setHttpPost(httpPostFunc);
    body->setup(armsFunc, iosFunc, sleepInterruptableFunc);
  }

  void ackCmd() {
    notifier->notificationRead();
    zCmd();
  }

  void zCmd() {
    body->performMove("Z.");
  }

  /**
   * Handle a user command.
   * Returns true if the command requires the current wait batch to be interrupted (normally true with change of bot mode)
   */
  bool command(const char *c) {
    if (strcmp("move", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULEB, Warn, "Argument needed:\n  move <move>");
        return false;
      }
      log(CLASS_MODULEB, Info, "-> Move %s", c);
      body->performMove(c);
      return false;
    } else if (strcmp("ack", c) == 0) {
    	ackCmd();
      log(CLASS_MODULEB, Info, "Notification read");
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
        logRaw(CLASS_MODULEB, Warn, "Arguments needed:\n  lcd <x> <y> <color> <wrap> <clear> <size> <str>");
        return false;
      }
      log(CLASS_MODULEB, Info, "-> Lcd %s", str);
      message(atoi(x), atoi(y), atoi(color), atoi(wrap), (MsgClearMode)atoi(clear), atoi(size), str);
      return false;
    } else if (strcmp("ifttttoken", c) == 0) {
      c = strtok(NULL, " ");
      if (c == NULL) {
        logRaw(CLASS_MODULEB, Warn, "Argument needed:\n  ifttttoken <token>");
        return false;
      }
      ifttt->setKey(c);
      log(CLASS_MODULEB, Info, "Ifttt token: %s", ifttt->getKey());
      return false;
    } else if (strcmp("help", c) == 0 || strcmp("?", c) == 0) {
      logRaw(CLASS_MODULE, Warn, HELP_COMMAND_CLI_PROJECT);
      module->command(c);
      return false;
    } else {
      log(CLASS_MODULE, Warn, "Not found in Botino: '%s'", c);
      return module->command(c);
    }
  }

  /**
   * Execute a command given an index
   *
   * Thought to be used via single button devices, so that
   * a button pressed can execute one of many available commands.
   */
  void sequentialCommand(int index, bool dryRun) {
    switch (index) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7: {
        int ind = index - 0;
        const char *mvName = getCommands()->getCmdName(ind);
        getNotifier()->message(0, 2, "%s?", mvName);
        if (!dryRun) {
          command(getCommands()->getCmdValue(ind));
        }
      } break;
      case 8: {
        getNotifier()->message(0, 2, "All act?");
        if (!dryRun) {
          module->actall();
          getNotifier()->message(0, 1, "All act one-off");
        }
      } break;
      case 9: {
        getNotifier()->message(0, 2, "Config mode?");
        if (!dryRun) {
          module->confCmd();
          getNotifier()->message(0, 1, "In config mode");
        }
      } break;
      case 10: {
        getNotifier()->message(0, 2, "Run mode?");
        if (!dryRun) {
          module->runCmd();
          getNotifier()->message(0, 1, "In run mode");
        }
      } break;
      case 11: {
        getNotifier()->message(0, 2, "Show info?");
        if (!dryRun) {
          module->infoCmd();
        }
      } break;
      default: {
        getNotifier()->message(0, 2, "Abort?");
        if (!dryRun) {
        	zCmd();
        }
      } break;
    }
  }

  Commands *getCommands() {
    return commands;
  }

  Body *getBody() {
    return body;
  }

  Ifttt *getIfttt() {
    return ifttt;
  }

  Notifier *getNotifier() {
    return notifier;
  }

  Settings *getSettings() {
    return module->getSettings();
  }

  Module *getModule() {
    return module;
  }

  SerBot *getBot() {
    return module->getBot();
  }

  void loop() {
  	module->loop();
  }
};

#endif // MODULE_BOTINO_INC
