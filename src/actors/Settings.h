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
  GlobalConfigStateDelimiter // delimiter of the configuration states
};

class Settings : public Actor {

private:
  const char *name;
  bool clear;
  int logLevel;
  int buttonPressed;
  Timing freqConf;

public:
  Settings(const char *n) : freqConf(OnceEvery5Seconds) {
    name = n;
    clear = false;
    logLevel = 0;
    buttonPressed = 0;
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
      default:
        return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (GlobalClearStackTraceState):
        if (setMode == SetValue) {
          Boolean b(targetValue);
          clear = b.get();
          log(CLASS_SETTINGS, Info, "%s: clear %d", name, clear);
        }
        if (actualValue != NULL) {
          Boolean b(clear);
          actualValue->load(&b);
        }
        break;
      case (GlobalLogLevelState):
        if (setMode == SetValue) {
          Integer b(targetValue);
          logLevel = b.get();
          log(CLASS_SETTINGS, Info, "%s: log %d", name, logLevel);
        }
        if (actualValue != NULL) {
          Integer b(logLevel);
          actualValue->load(&b);
        }
        break;
      case (GlobalButtonPressedState):
        if (setMode == SetValue) {
          Integer b(targetValue);
          buttonPressed = b.get();
        }
        if (actualValue != NULL) {
          Integer b(buttonPressed);
          actualValue->load(&b);
        }
        break;
      default:
        break;
    }
  }

  int getNroProps() {
    return GlobalConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {
    Boolean i(clear);
    info->load(&i);
  }

  int getNroInfos() {
    return 1;
  }

  Timing *getFrequencyConfiguration() {
    return &freqConf;
  }

  bool getClear() {
    return clear;
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

};

#endif // GLOBAL_INC
