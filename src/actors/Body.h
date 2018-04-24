#ifndef BODY_INC
#define BODY_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_BODY "BO"
#define LCD_LINE_LENGTH 32
#define SEQUENCE_STR_LENGTH (2 * 8)

#define SEQVAL(a, b) (int)(((int)(a)) * 256 + (b))

enum BodyConfigState {
  BodyConfigMsg = 0,         // message
  BodyConfigSeq0,           // sequence
  BodyConfigTime,           // time of acting
  BodyConfigCron,           // cron of acting
  BodyConfigStateDelimiter // delimiter of the configuration states
};

enum ArmState { ArmUp = 0, ArmMiddle, ArmDown, ArmDelimiter };

class Body : public Actor {

private:
  const char *name;
  Timing freqConf;
  void (*smilyFace)();
  void (*sadFace)();
  void (*normalFace)();
  void (*sleepyFace)();
  void (*arms)(ArmState left, ArmState right);
  void (*messageFunc)(int line, const char *msg);
  Buffer<LCD_LINE_LENGTH> *msg;
  Buffer<SEQUENCE_STR_LENGTH> *sequence0;
  long time;
  long cron;

  bool isInitialized() {
    return smilyFace != NULL && sadFace != NULL && normalFace != NULL && sleepyFace != NULL && arms != NULL && messageFunc != NULL;
  }

  void actSub(char c1, char c2) {
    switch (SEQVAL(c1, c2)) {
    	// faces
      case SEQVAL('f', 's'):
        smilyFace();
        break;
      case SEQVAL('f', 'S'):
        sadFace();
        break;
      case SEQVAL('f', 'n'):
        normalFace();
        break;
      case SEQVAL('f', 'l'):
        sleepyFace();
        break;
      // arms
      case SEQVAL('a', 'u'):
        arms(ArmUp, ArmUp);
        break;
      case SEQVAL('a', 'r'):
        arms(ArmDown, ArmUp);
        break;
      case SEQVAL('a', 'l'):
        arms(ArmUp, ArmDown);
        break;
      case SEQVAL('a', 'd'):
        arms(ArmDown, ArmDown);
        break;
      // messages
      case SEQVAL('m', '1'):
        messageFunc(0, msg->getBuffer());
        break;
      // misc
      case SEQVAL('w', 'a'):
        delay(1000);
        break;
      // default
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
    time = 0L;
    cron = 0L;
    msg = new Buffer<LCD_LINE_LENGTH>("msg");
    sequence0 = new Buffer<SEQUENCE_STR_LENGTH>("m1auadfsfn");
  }

  const char *getName() {
    return name;
  }

  void setSleepyFace(void (*f)()) {
    sleepyFace = f;
  }
  void setNormalFace(void (*f)()) {
    normalFace = f;
  }
  void setSmilyFace(void (*f)()) {
    smilyFace = f;
  }
  void setSadFace(void (*f)()) {
    sadFace = f;
  }
  void setArms(void (*f)(ArmState left, ArmState right)) {
    arms = f;
  }
  void setMessageFunc(void (*f)(int line, const char *str)) {
    messageFunc = f;
  }

  void act() {
    if (!isInitialized()) {
      return;
    }
    if (freqConf.matches()) {
      const char* s = sequence0->getBuffer();
    	for (int i = 0; i < strlen(s); i = i + 2) {
        actSub(s[i], s[i + 1]);
    	}

    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (BodyConfigMsg):
        return "ms";
      case (BodyConfigSeq0):
        return "s0";
      case (BodyConfigTime):
        return "ti";
      case (BodyConfigCron):
        return "cr";
      default:
        return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (BodyConfigMsg):
        setPropValue(setMode, targetValue, actualValue, msg);
        break;
      case (BodyConfigSeq0):
        setPropValue(setMode, targetValue, actualValue, sequence0);
        break;
      case (BodyConfigTime):
        setPropLong(setMode, targetValue, actualValue, (long*)&time);
        if (setMode == SetValue) {
          freqConf.setCustom(time);
          freqConf.setFrequency(CustomMoment);
        }
        break;
      case (BodyConfigCron):
        setPropLong(setMode, targetValue, actualValue, (long*)&cron);
        if (setMode == SetValue) {
          freqConf.setCron(cron);
          freqConf.setFrequency(CustomCron);
        }
        break;
      default:
        break;
    }
  }

  int getNroProps() {
    return BodyConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &freqConf;
  }
};

#endif // BODY_INC
