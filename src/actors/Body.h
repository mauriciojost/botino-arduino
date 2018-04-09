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
  Happy = 0,
  Sad,
  Normal,
  BodyStateDelimiter
};

// Face
// - Smily
// - Sad
// Body
// - Arms up
// - Arms down
// - Arms shaking

class Body : public Actor {

private:

  const char *name;
  Timing freqConf;
  void (*smile)();
  void (*beSad)();
  void (*arms)(ArmState left, ArmState right);
  Mood mood;

  bool isInitialized() {
  	return smile != NULL && beSad != NULL && arms != NULL;
  }

public:

  Body(const char *n) : freqConf(OnceEvery5Seconds) {
    name = n;
    smile = NULL;
    beSad = NULL;
    arms = NULL;
    mood = Normal;
  }

  const char *getName() {
    return name;
  }

  void setSmile(void (*f)()) { smile = f; }
  void setBeSad(void (*f)()) { beSad = f; }
  void setArms(void (*f)(ArmState left, ArmState right)) { arms = f; }

  void actMood() {
  	switch (mood) {
  		case Happy:
        smile();
        arms(ArmUp, ArmUp);
        beSad();
        arms(ArmDown, ArmDown);
        smile();
        arms(ArmUp, ArmUp);
        arms(ArmDown, ArmDown);
        arms(ArmUp, ArmUp);
        arms(ArmDown, ArmDown);
        arms(ArmUp, ArmUp);
        smile();
        break;
  		default:
  			break;
  	}
  }

  void act() {
  	if (!isInitialized()) {
  		return;
  	}
    if (freqConf.matches()) {
    	actMood();
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
