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
  GlobalShowSettings = 0,
  GlobalClearStackTraceState,
  GlobalLogLevelState,
  GlobalButtonPressedState,
  GlobalPeriodSecondsState,
  GlobalDisableLcdState,
  GlobalConfigStateDelimiter // delimiter of the configuration states
};

class Settings : public Actor {

private:
  const char *name;
  bool showSettings;
  bool clearStackTrace;
  int logLevel;
  int buttonPressed;
  int periodSeconds;
  bool disableLcd;
  Timing freqConf;

public:
  Settings(const char *n) : freqConf(Never) {
    name = n;
    showSettings = true;
    clearStackTrace = false;
    logLevel = 0;
    buttonPressed = 0;
    periodSeconds = PERIOD_SEC;
    disableLcd = true;
  }

  const char *getName() {
    return name;
  }

  void act() {}

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (GlobalShowSettings):
        return "sh";
      case (GlobalClearStackTraceState):
        return "cl";
      case (GlobalLogLevelState):
        return "ll";
      case (GlobalButtonPressedState):
        return "bp";
      case (GlobalPeriodSecondsState):
        return "ps";
      case (GlobalDisableLcdState):
        return "di";
      default:
        return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (GlobalShowSettings):
        setPropBoolean(setMode, targetValue, actualValue, &showSettings);
        break;
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
      case (GlobalDisableLcdState):
        setPropBoolean(setMode, targetValue, actualValue, &disableLcd);
        break;
      default:
        break;
    }
  }

  int getNroProps() {
    if (showSettings) {
      return GlobalConfigStateDelimiter;
    } else {
      return (GlobalShowSettings + 1);
    }
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

  int getPeriodSeconds() {
    return periodSeconds;
  }

  bool getDisableLcd() {
    return disableLcd;
  }
};

#endif // GLOBAL_INC
