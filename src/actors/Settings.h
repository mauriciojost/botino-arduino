#ifndef GLOBAL_INC
#define GLOBAL_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_SETTINGS "ST"

#ifndef PERIOD_SEC
#define PERIOD_SEC 30
#endif // PERIOD_SEC

enum GlobalConfigState {
  GlobalClearStackTraceState = 0,
  GlobalLogLevelState,
  GlobalButtonPressedState,
  GlobalPeriodSecondsState,
  GlobalLcdDebugState,
  GlobalConfigStateDelimiter // delimiter of the configuration states
};

class Settings : public Actor {

private:
  const char *name;
  bool clearStackTrace;
  int logLevel;
  int buttonPressed;
  int periodSeconds;
  bool lcdDebug;
  Timing freqConf;

public:
  Settings(const char *n) : freqConf(Never) {
    name = n;
    clearStackTrace = false;
    logLevel = 0;
    buttonPressed = 0;
    periodSeconds = PERIOD_SEC;
    lcdDebug = true;
  }

  const char *getName() {
    return name;
  }

  void act() {}

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (GlobalClearStackTraceState):
        return "cl";
      case (GlobalLogLevelState):
        return "logl";
      case (GlobalButtonPressedState):
        return "bp";
      case (GlobalPeriodSecondsState):
        return "period";
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
      case (GlobalButtonPressedState):
        setPropInteger(setMode, targetValue, actualValue, &buttonPressed);
        break;
      case (GlobalPeriodSecondsState):
        setPropInteger(setMode, targetValue, actualValue, &periodSeconds);
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

  int getButtonPressed() {
    return buttonPressed;
  }

  void setButtonPressed(int v) {
    buttonPressed = v;
  }

  void incrButtonPressed(int v) {
    buttonPressed += v;
  }

  int getPeriodSeconds() {
    return periodSeconds;
  }

  bool getLcdDebug() {
    return lcdDebug;
  }
};

#endif // GLOBAL_INC
