#ifndef GLOBAL_INC
#define GLOBAL_INC

/**
 * Settings
 *
 * Holds global settings for the device (like log level, debug mode, etc.).
 *
 */

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Integer.h>
#include <main4ino/Value.h>

#define CLASS_SETTINGS "ST"

enum SettingsProps {
  SettingsClearStackTraceProp = 0, // boolean, clear the stack trace log if full
  SettingsLogLevelProp,            // integer, define the log level
  SettingsLcdDebugProp,            // boolean, define if the LCD shows the debug logs
  SettingsButtonRoutineLimitProp,  // integer, define the first X routines that are randomly executed when the button is pressed
  SettingsPropsDelimiter           // amount of properties
};

class Settings : public Actor {

private:
  const char *name;
  bool clearStackTrace;
  int logLevel;
  bool lcdDebug;
  int buttonRoutineUntil;
  Timing freqConf;

public:
  Settings(const char *n) {
    name = n;
    clearStackTrace = false;
    logLevel = 0;
    lcdDebug = true;
    buttonRoutineUntil = 1;
    freqConf.setFrequency(Never);
  }

  const char *getName() {
    return name;
  }

  void act() {}

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (SettingsClearStackTraceProp):
        return "crashclear";
      case (SettingsLogLevelProp):
        return "loglevel";
      case (SettingsLcdDebugProp):
        return "lcddebug";
      case (SettingsButtonRoutineLimitProp):
        return "btnrout";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (SettingsClearStackTraceProp):
        setPropBoolean(m, targetValue, actualValue, &clearStackTrace);
        break;
      case (SettingsLogLevelProp):
        setPropInteger(m, targetValue, actualValue, &logLevel);
        break;
      case (SettingsLcdDebugProp):
        setPropBoolean(m, targetValue, actualValue, &lcdDebug);
        break;
      case (SettingsButtonRoutineLimitProp):
        setPropInteger(m, targetValue, actualValue, &buttonRoutineUntil);
        break;
      default:
        break;
    }
  }

  int getNroProps() {
    return SettingsPropsDelimiter;
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

  int getNroRoutinesForButton() {
    return buttonRoutineUntil;
  }

  bool getLcdDebug() {
    return lcdDebug;
  }
};

#endif // GLOBAL_INC
