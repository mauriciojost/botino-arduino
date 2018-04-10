#ifndef BODY_INC
#define BODY_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_BODY "BO"

enum BodyConfigState {
  BodyConfigMood = 0,   // if the led is on
  BodyConfigStateDelimiter // delimiter of the configuration states
};

enum ArmState {
  ArmUp = 0,
  ArmMiddle,
  ArmDown,
  ArmDelimiter
};

enum Mood {
  Normal = 0,
  Sad,
  Happy,
  Sleepy,
  BodyStateDelimiter
};

class Body : public Actor {

private:

  const char *name;
  Timing freqConf;
  void (*smilyFace)();
  void (*sadFace)();
  void (*normalFace)();
  void (*sleepyFace)();
  void (*arms)(ArmState left, ArmState right);
  Mood mood;

  bool isInitialized() {
  	return smilyFace != NULL && sadFace != NULL && normalFace != NULL && sleepyFace != NULL && arms != NULL;
  }

  void actMood() {
  	switch (mood) {
  		case Happy:
        smilyFace();
        arms(ArmUp, ArmUp);
        arms(ArmDown, ArmDown);
        arms(ArmUp, ArmUp);
        arms(ArmDown, ArmDown);
        arms(ArmUp, ArmUp);
        smilyFace();
        break;
  		case Sad:
        sadFace();
        arms(ArmUp, ArmUp);
        arms(ArmMiddle, ArmMiddle);
        arms(ArmDown, ArmDown);
        arms(ArmMiddle, ArmMiddle);
        arms(ArmDown, ArmDown);
        sadFace();
        break;
  		case Normal:
        normalFace();
        arms(ArmDown, ArmDown);
        break;
  		case Sleepy:
        sleepyFace();
        arms(ArmDown, ArmDown);
        arms(ArmMiddle, ArmMiddle);
        arms(ArmDown, ArmDown);
        sleepyFace();
        break;
  		default:
  			break;
  	}
  }


public:

  Body(const char *n) : freqConf(OnceEvery10Seconds) {
    name = n;
    smilyFace = NULL;
    sadFace = NULL;
    normalFace = NULL;
    sleepyFace = NULL;
    arms = NULL;
    mood = Happy;
  }

  const char *getName() {
    return name;
  }

  void setSleepyFace(void (*f)()) { sleepyFace = f; }
  void setNormalFace(void (*f)()) { normalFace = f; }
  void setSmilyFace(void (*f)()) { smilyFace = f; }
  void setSadFace(void (*f)()) { sadFace = f; }
  void setArms(void (*f)(ArmState left, ArmState right)) { arms = f; }

  void act() {
  	if (!isInitialized()) {
  		return;
  	}
    if (freqConf.matches()) {
    	actMood();
    	mood = Normal;
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (BodyConfigMood):
        return "mo";
      default:
        return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (BodyConfigMood):
      	setPropInteger(setMode, targetValue, actualValue, (int*)&mood);
        break;
      default:
      	break;
    }
  }

  int getNroProps() { return BodyConfigStateDelimiter; }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) { }

  int getNroInfos() { return 0; }

  Timing *getFrequencyConfiguration() { return &freqConf; }

};

#endif // BODY_INC
