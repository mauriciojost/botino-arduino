#ifndef BODY_INC
#define BODY_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_BODY "BO"
#define LCD_LINE_LENGTH 32

enum BodyConfigState {
  BodyConfigMood = 0,      // if the led is on
  BodyConfigMsg,           // message
  BodyConfigStateDelimiter // delimiter of the configuration states
};

enum ArmState {
  ArmUp = 0,
  ArmMiddle,
  ArmDown,
  ArmDelimiter
};

enum Mood {
  Sleepy = 0,
  Normal,
  Sad,
  Happy,
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
  void (*messageFunc)(int line, const char* msg);
  Buffer<LCD_LINE_LENGTH> *msg;
  Mood mood;

  bool isInitialized() {
  	return smilyFace != NULL && sadFace != NULL && normalFace != NULL && sleepyFace != NULL && arms != NULL && messageFunc != NULL;
  }

  void actMood() {
  	switch (mood) {
  		case Happy:
        arms(ArmUp, ArmUp);
  			messageFunc(0, msg->getBuffer());
        smilyFace();
  			messageFunc(0, msg->getBuffer());
        smilyFace();
        arms(ArmUp, ArmUp);
        arms(ArmDown, ArmDown);
        arms(ArmUp, ArmUp);
        arms(ArmDown, ArmDown);
        arms(ArmUp, ArmUp);
        arms(ArmDown, ArmDown);
        smilyFace();
        break;
  		case Sad:
        arms(ArmUp, ArmUp);
  			messageFunc(0, msg->getBuffer());
        sadFace();
  			messageFunc(0, msg->getBuffer());
        sadFace();
        arms(ArmUp, ArmUp);
        arms(ArmMiddle, ArmMiddle);
        arms(ArmDown, ArmDown);
        arms(ArmMiddle, ArmMiddle);
        arms(ArmDown, ArmDown);
        sadFace();
        break;
  		case Normal:
        arms(ArmMiddle, ArmMiddle);
  			messageFunc(0, msg->getBuffer());
        normalFace();
  			messageFunc(0, msg->getBuffer());
        normalFace();
        arms(ArmDown, ArmDown);
        break;
  		case Sleepy:
  			messageFunc(0, msg->getBuffer());
        sleepyFace();
  			messageFunc(0, msg->getBuffer());
        sleepyFace();
        arms(ArmDown, ArmDown);
        sleepyFace();
        break;
  		default:
  			break;
  	}
  }


public:

  Body(const char *n) : freqConf(OnceEvery1Minute) {
    name = n;
    smilyFace = NULL;
    sadFace = NULL;
    normalFace = NULL;
    sleepyFace = NULL;
    arms = NULL;
    messageFunc = NULL;
    mood = Sleepy;
    msg = new Buffer<LCD_LINE_LENGTH>("");
  }

  const char *getName() {
    return name;
  }

  void setSleepyFace(void (*f)()) { sleepyFace = f; }
  void setNormalFace(void (*f)()) { normalFace = f; }
  void setSmilyFace(void (*f)()) { smilyFace = f; }
  void setSadFace(void (*f)()) { sadFace = f; }
  void setArms(void (*f)(ArmState left, ArmState right)) { arms = f; }
  void setMessageFunc(void (*f)(int line, const char* str)) { messageFunc = f; }

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
      case (BodyConfigMsg):
        return "ms";
      default:
        return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (BodyConfigMood):
      	setPropInteger(setMode, targetValue, actualValue, (int*)&mood);
        break;
      case (BodyConfigMsg):
      	setPropValue(setMode, targetValue, actualValue, msg);
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
