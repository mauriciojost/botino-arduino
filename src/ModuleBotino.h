#ifndef MODULE_BOTINO_INC
#define MODULE_BOTINO_INC

#include <Constants.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>

#include <actors/Body.h>
#include <actors/BotinoSettings.h>
#include <actors/Commands.h>
#include <actors/Ifttt.h>
#include <actors/Images.h>
#include <actors/Notifier.h>
#include <actors/Quotes.h>
#include <mod4ino/Module.h>

#define CLASS_MODULEB "MB"

#define COMMAND_MAX_LENGTH 128
#define PROPSYNC_RETRY_PERIOD_SECS 600

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
  // Main4ino Module
  Module *module;

  // Actors
  Body *body;
  Quotes *quotes;
  Images *images;
  Ifttt *ifttt;
  Notifier *notifier;
  Commands *commands;
  BotinoSettings *bsettings;

  void (*message)(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str);

  std::function<CmdExecStatus (Cmd* cmd)> cmdProjExtFunc = [&](Cmd* c) {
      return commandProjectExtended(c);
  };


public:
  ModuleBotino() {

    module = new Module();

    bsettings = new BotinoSettings("botino");
    quotes = new Quotes("quotes");
    body = new Body("body");
    images = new Images("images");
    ifttt = new Ifttt("ifttt");
    notifier = new Notifier("notifier");
    commands = new Commands("commands");

    module->getActors()->add(
        7, (Actor *)bsettings, (Actor *)quotes, (Actor *)body, (Actor *)images, (Actor *)ifttt, (Actor *)notifier, (Actor *)commands);

    message = NULL;
  }

  void setup(void (*lcdImgFunc)(char img, uint8_t bitmap[]),
             void (*armsFunc)(int left, int right, int steps),
             void (*messageFunc)(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str),
             void (*iosFunc)(char led, IoMode v),
             std::function<bool ()> initWifiFunc,
             std::function<void ()> stopWifiFunc,
             HttpResponse (*httpMethodFunc)(HttpMethod m, const char *url, Stream *body, Table *headers, const char *fingerprint),
             void (*clearDeviceFunc)(),
             bool (*fileReadFunc)(const char *fname, Buffer *content),
             bool (*fileWriteFunc)(const char *fname, const char *content),
             bool (*sleepInterruptableFunc)(time_t cycleBegin, time_t periodSec),
             void (*deepSleepNotInterruptableFunc)(time_t cycleBegin, time_t periodSec),
             void (*configureModeArchitectureFunc)(),
             void (*runModeArchitectureFunc)(),
             CmdExecStatus (*commandArchitectureFunc)(Cmd *cmd),
             void (*infoFunc)(),
             void (*updateFunc)(const char *, const char *),
             void (*testFunc)(),
             const char *(*apiDeviceLoginFunc)(),
             const char *(*apiDevicePassFunc)(),
             Buffer *(*getLogBufferFunc)(),
             bool (*buttonIsPressedFunc)()) {

    module->setup(STRINGIFY(PROJECT_ID),
                  STRINGIFY(PLATFORM_ID),
                  initWifiFunc,
                  stopWifiFunc,
                  httpMethodFunc,
                  clearDeviceFunc,
                  fileReadFunc,
                  fileWriteFunc,
                  sleepInterruptableFunc,
                  deepSleepNotInterruptableFunc,
                  configureModeArchitectureFunc,
                  runModeArchitectureFunc,
                  commandArchitectureFunc,
                  cmdProjExtFunc,
                  infoFunc,
                  updateFunc,
                  testFunc,
                  apiDeviceLoginFunc,
                  apiDevicePassFunc,
                  NULL,
                  getLogBufferFunc);

    message = messageFunc;

    body->setQuotes(quotes);
    body->setImages(images);
    body->setIfttt(ifttt);
    body->setNotifier(notifier);

    notifier->setup(lcdImgFunc, messageFunc);
    quotes->setHttpMethod(httpMethodFunc);
    quotes->setInitWifi(initWifiFunc);
    ifttt->setInitWifi(initWifiFunc);
    ifttt->setHttpMethod(httpMethodFunc);
    body->setup(armsFunc, iosFunc, sleepInterruptableFunc, buttonIsPressedFunc);
  }

  StartupStatus startupProperties() {
    StartupStatus result = module->startupProperties();
    module->getPropSync()->getTiming()->setRetryPeriod(PROPSYNC_RETRY_PERIOD_SECS);
    return result;
  }

  StartupStatus startupPropertiesLight() {
    StartupStatus result = module->startupPropertiesLight();
    module->getPropSync()->getTiming()->setRetryPeriod(PROPSYNC_RETRY_PERIOD_SECS);
    return result;
  }

  void ackCmd() {
    notifier->notificationRead();
    zCmd();
  }

  void zCmd() {
    body->z();
  }

 /**
   * Handle a user command.
   */
  CmdExecStatus commandProjectExtended(Cmd *c) {
    // HEADS UP: changing the frequency to 160 MHz makes the servo module work unpredictably.
    if (c->matches("move", "perform a move", 1, "move")) {
      log(CLASS_MODULEB, Info, "-> move %s", c->getAsLastArg(0));
      body->performMove(c->getAsLastArg(0));
      return Executed;
    } else if (c->matches("ack", "acknoledge notification", 0)) {
      ackCmd();
      log(CLASS_MODULEB, Info, "Notification read");
      return Executed;
    } else if (c->matches("lcd", "write message to LCD", 7, "x", "y", "color", "wrap", "clear", "size", "str")) {
      int x = c->getArgIntBE(0);
      int y = c->getArgIntBE(1);
      int color = c->getArgIntBE(2);
      int wrap = c->getArgIntBE(3);
      int clear = c->getArgIntBE(4);
      int size = c->getArgIntBE(5);
      const char *str = c->getAsLastArg(6);
      log(CLASS_MODULEB, User, "-> Lcd %s", str);
      message(x, y, color, wrap, (MsgClearMode)clear, size, str);
      return Executed;
    } else if (c->matches("ifttttoken", "provide ifttt token", 1, "token") == 0) {
      ifttt->setKey(c->getAsLastArg(0));
      log(CLASS_MODULEB, Info, "Ifttt token: %s", ifttt->getKey());
      return Executed;
    } else {
      return NotFound;
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
          // command(getCommands()->getCmdValue(ind)); TODO FIX
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

  Settings *getModuleSettings() {
    return module->getSettings();
  }

  BotinoSettings *getBotinoSettings() {
    return bsettings;
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
