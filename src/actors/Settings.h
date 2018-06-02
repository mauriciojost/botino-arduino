#ifndef GLOBAL_INC
#define GLOBAL_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_SETTINGS "ST"

enum GlobalConfigState {
  GlobalClearStackTraceState = 0,
  GlobalLogLevelState,
  GlobalLcdDebugState,
  GlobalConfigStateDelimiter // delimiter of the configuration states
};

class Settings : public Actor {

private:
  const char *name;
  bool clearStackTrace;
  int logLevel;
  bool lcdDebug;
  Timing freqConf;

public:
  Settings(const char *n) : freqConf(Never) {
    name = n;
    clearStackTrace = false;
    logLevel = 0;
    lcdDebug = true;
  }

  const char *getName() {
    return name;
  }

  void act() {}

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (GlobalClearStackTraceState):
        return "crashclear";
      case (GlobalLogLevelState):
        return "loglevel";
      case (GlobalLcdDebugState):
        return "lcddebug";
      default:
        return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (GlobalClearStackTraceState):
        setPropBoolean(setMode, targetValue, actualValue, &clearStackTrace);
        break;
      case (GlobalLogLevelState):
        setPropInteger(setMode, targetValue, actualValue, &logLevel);
        break;
      case (GlobalLcdDebugState):
        setPropBoolean(setMode, targetValue, actualValue, &lcdDebug);
        break;
      default:
        break;
    }
  }

  int getNroProps() {
    return GlobalConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &freqConf;
  }

  bool getClear() {
    return clearStackTrace;
  }

  int getLogLevel() {
    return logLevel;
  }

  bool getLcdDebug() {
    return lcdDebug;
  }
};

#endif // GLOBAL_INC
