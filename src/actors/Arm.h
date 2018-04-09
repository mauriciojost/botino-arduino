#ifndef ARM_INC
#define ARM_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_ARM "AR"

enum ArmConfigState {
  ArmConfigOnState = 0,
  ArmConfigStateDelimiter // delimiter of the configuration states
};

class Arm : public Actor {

private:
  const char *name;
  int currentPosition;
  Timing freqConf;
  void (*setPositionFunction)(int);

public:
  Arm(const char *n) : freqConf(Never) {
    name = n;
    currentPosition = 0;
    setPositionFunction = NULL;
  }

  const char *getName() {
    return name;
  }

  void setServoPositionFunction(void (*f)(int)) {
    setPositionFunction = f;
  }

  void act() {
    if (freqConf.matches()) {
      if (setPositionFunction != NULL) {
        log(CLASS_ARM, Info, "%s set: %d", name, currentPosition);
        setPositionFunction(currentPosition);
      }
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (ArmConfigOnState):
        return "pos";
      default:
        return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (ArmConfigOnState):
        if (setMode == SetValue) {
          Integer b(targetValue);
          currentPosition = b.get();
        }
        if (setMode != DoNotSet) {
          log(CLASS_ARM, Info, "Arm: %s", name);
          log(CLASS_ARM, Info, " set: %d", currentPosition);
        }
        if (actualValue != NULL) {
          Integer b(currentPosition);
          actualValue->load(&b);
        }
        break;
      default:
        break;
    }
  }

  int getNroProps() {
    return ArmConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {
    Integer i(currentPosition);
    info->load(&i);
  }

  int getNroInfos() {
    return 1;
  }

  Timing *getFrequencyConfiguration() {
    return &freqConf;
  }
};

#endif // ARM_INC
