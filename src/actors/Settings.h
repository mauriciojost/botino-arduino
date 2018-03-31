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
  GlobalButtonPressedState,
  GlobalPeriodSecondsState,
  GlobalDisableLcdState,
  GlobalConfigStateDelimiter // delimiter of the configuration states
};

class Settings : public Actor {

private:
  const char *name;
  bool clearStackTrace;
  int logLevel;
  int buttonPressed;
  int periodSeconds;
  bool disableLcd;
  Timing freqConf;

public:
  Settings(const char *n) : freqConf(Never) {
    name = n;
    clearStackTrace = false;
    logLevel = 1;
    buttonPressed = 0;
    periodSeconds = 1;
    disableLcd = true;
  }

  const char *getName() {
    return name;
  }

  void cycle() {}

  const char *getPropName(int propIndex) {
    switch (propIndex) {
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
      case (GlobalClearStackTraceState):
      	setPropBoolean(setMode, targetValue, actualValue, &clearStackTrace); break;
      case (GlobalLogLevelState):
      	setPropInteger(setMode, targetValue, actualValue, &logLevel); break;
      case (GlobalButtonPressedState):
      	setPropInteger(setMode, targetValue, actualValue, &buttonPressed); break;
      case (GlobalPeriodSecondsState):
      	setPropInteger(setMode, targetValue, actualValue, &periodSeconds); break;
      case (GlobalDisableLcdState):
      	setPropBoolean(setMode, targetValue, actualValue, &disableLcd); break;
      default:
        break;
    }
  }

  int getNroProps() {
    return GlobalConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {
    Boolean i(clearStackTrace);
    info->load(&i);
  }

  int getNroInfos() {
    return 1;
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
