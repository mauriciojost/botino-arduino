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

#define INFO_BUFFER_LENGTH 256

enum SettingsProps {
  SettingsDebugProp,              // boolean, define if the device is in debug mode
  SettingsButtonRoutineLimitProp, // integer, define the first X routines that are randomly executed when the button is pressed
  SettingsPropsDelimiter          // amount of properties
};

class Settings : public Actor {

private:
  const char *name;
  bool devDebug;
  int buttonRoutineUntil;
  Buffer* infoBuffer;
  Metadata* md;

public:
  Settings(const char *n) {
    name = n;
    infoBuffer = new Buffer(INFO_BUFFER_LENGTH);
    devDebug = false;
    buttonRoutineUntil = 4;
    infoBuffer->clear();
    md = new Metadata(n);
  }

  const char *getName() {
    return name;
  }

  void act() {}

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (SettingsDebugProp):
        return "debug";
      case (SettingsButtonRoutineLimitProp):
        return "btnrout";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (SettingsDebugProp):
        setPropBoolean(m, targetValue, actualValue, &devDebug);
        break;
      case (SettingsButtonRoutineLimitProp):
        setPropInteger(m, targetValue, actualValue, &buttonRoutineUntil);
        break;
      default:
        break;
    }
    if (m != GetValue) {
    	getMetadata()->changed();
    }
  }

  int getNroProps() {
    return SettingsPropsDelimiter;
  }

  void getInfo(int infoIndex, Buffer *info) {
    info->load(infoBuffer);
  }

  int getNroInfos() {
    return 1;
  }

  Metadata *getMetadata() {
    return md;
  }

  int getNroRoutinesForButton() {
    return buttonRoutineUntil;
  }

  bool getDebug() {
    return devDebug;
  }

  void setDebug(bool b) {
  	if (b != devDebug) {
      getMetadata()->changed();
  	}
    devDebug = b;
  }

  void setInfo(const char *s) {
    infoBuffer->fill(s);
  }
};

#endif // GLOBAL_INC
