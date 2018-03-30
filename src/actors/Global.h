#ifndef GLOBAL_INC
#define GLOBAL_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_GLOBAL "GL"

enum GlobalConfigState {
  GlobalClearStackTraceState = 0,
  GlobalLogLevelState,
  GlobalConfigStateDelimiter // delimiter of the configuration states
};

class Global : public Actor {

private:
  const char* name;
  bool clear;
  int logLevel;
  Timing freqConf;

public:

  Global(const char* n): freqConf(OnceEvery5Seconds) {
    name = n;
    clear = false;
    logLevel = 0;
  }

  const char *getName() {
    return name;
  }

  void cycle() { }

  const char* getPropName(int propIndex) {
    switch (propIndex) {
      case (GlobalClearStackTraceState): return "cl";
      case (GlobalLogLevelState): return "ll";
      default: return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value* targetValue, Value* actualValue) {
    switch (propIndex) {
      case (GlobalClearStackTraceState):
        if (setMode == SetValue) {
          Boolean b(targetValue);
          clear = b.get();
        }
        if (setMode != DoNotSet) {
          log(CLASS_GLOBAL, Info, "Global: %s", name);
          log(CLASS_GLOBAL, Info, " clear: %d", clear);
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
        }
        if (setMode != DoNotSet) {
          log(CLASS_GLOBAL, Info, "Global: %s", name);
          log(CLASS_GLOBAL, Info, " log: %d", logLevel);
        }
        if (actualValue != NULL) {
          Integer b(logLevel);
          actualValue->load(&b);
        }
        break;
      default:
        break;
    }
  }

  int getNroProps() { return GlobalConfigStateDelimiter; }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH>* info) {
    Boolean i(clear);
    info->load(&i);
  }

  int getNroInfos() { return 1; }

  Timing *getFrequencyConfiguration() { return &freqConf; }

  bool getClear() { return clear; }
  int getLogLevel() { return logLevel; }

};

#endif // GLOBAL_INC
