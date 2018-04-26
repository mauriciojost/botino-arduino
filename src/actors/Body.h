#ifndef BODY_INC
#define BODY_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_BODY "BO"
#define MSG_MAX_LENGTH 32
#define MAX_POSI_PER_MOVE 10 // maximum amount of positions per move
#define POSI_STR_LENGTH 3 // characters that represent a position / state within a move
#define MOVE_STR_LENGTH (POSI_STR_LENGTH * MAX_POSI_PER_MOVE)

#define SEQVAL(a, b) (int)(((int)(a)) * 256 + (b))

enum BodyConfigState {
  BodyConfigMsg0 = 0,         // message 0
  BodyConfigMsg1,             // message 1
  BodyConfigMsg2,             // message 2
  BodyConfigMsg3,             // message 3
  BodyConfigMove,            // move
  BodyConfigTime,             // time of acting
  BodyConfigCron,             // cron of acting
  BodyConfigStateDelimiter    // delimiter of the configuration states
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
  Buffer<MSG_MAX_LENGTH> *msg0;
  Buffer<MSG_MAX_LENGTH> *msg1;
  Buffer<MSG_MAX_LENGTH> *msg2;
  Buffer<MSG_MAX_LENGTH> *msg3;
  Buffer<MOVE_STR_LENGTH> *move;
  long time;
  long cron;

  bool isInitialized() {
    return smilyFace != NULL && sadFace != NULL && normalFace != NULL && sleepyFace != NULL && arms != NULL && messageFunc != NULL;
  }

  void actSub(char c1, char c2) {
    switch (SEQVAL(c1, c2)) {
    	// faces
      case SEQVAL('f', 's'):
        log(CLASS_BODY, Debug, "Smile");
        smilyFace();
        break;
      case SEQVAL('f', 'S'):
        log(CLASS_BODY, Debug, "Sad");
        sadFace();
        break;
      case SEQVAL('f', 'n'):
        log(CLASS_BODY, Debug, "Normal");
        normalFace();
        break;
      case SEQVAL('f', 'l'):
        log(CLASS_BODY, Debug, "Sleepy");
        sleepyFace();
        break;
      // arms
      case SEQVAL('a', 'u'):
        log(CLASS_BODY, Debug, "Arms up");
        arms(ArmUp, ArmUp);
        break;
      case SEQVAL('a', 'r'):
        log(CLASS_BODY, Debug, "Arms down/up");
        arms(ArmDown, ArmUp);
        break;
      case SEQVAL('a', 'l'):
        log(CLASS_BODY, Debug, "Arms up/down");
        arms(ArmUp, ArmDown);
        break;
      case SEQVAL('a', 'd'):
        log(CLASS_BODY, Debug, "Arms down");
        arms(ArmDown, ArmDown);
        break;
      // messages
      case SEQVAL('m', '0'):
        log(CLASS_BODY, Debug, "Message 0");
        messageFunc(0, msg0->getBuffer());
        break;
      case SEQVAL('m', '1'):
        log(CLASS_BODY, Debug, "Message 1");
        messageFunc(0, msg1->getBuffer());
        break;
      case SEQVAL('m', '2'):
        log(CLASS_BODY, Debug, "Message 2");
        messageFunc(0, msg2->getBuffer());
        break;
      case SEQVAL('m', '3'):
        log(CLASS_BODY, Debug, "Message 3");
        messageFunc(0, msg3->getBuffer());
        break;
      // misc
      case SEQVAL('w', 'a'):
        log(CLASS_BODY, Debug, "Wait");
        delay(1000);
        break;
      // default
      default:
        log(CLASS_BODY, Debug, "Invalid command");
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
    msg0 = new Buffer<MSG_MAX_LENGTH>("0");
    msg1 = new Buffer<MSG_MAX_LENGTH>("1");
    msg2 = new Buffer<MSG_MAX_LENGTH>("2");
    msg3 = new Buffer<MSG_MAX_LENGTH>("3");
    move = new Buffer<MOVE_STR_LENGTH>("");
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
      log(CLASS_BODY, Debug, "Body up");
      const char* s = move->getBuffer();
      log(CLASS_BODY, Debug, "Seq: %s", s);
      for (int i = 0; i < strlen(s); i+=2) {
        actSub(s[i], s[i + 1]);
      }

    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (BodyConfigMsg0):
        return "m0";
      case (BodyConfigMsg1):
        return "m1";
      case (BodyConfigMsg2):
        return "m2";
      case (BodyConfigMsg3):
        return "m3";
      case (BodyConfigMove):
        return "mo";
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
      case (BodyConfigMsg0):
        setPropValue(setMode, targetValue, actualValue, msg0);
        break;
      case (BodyConfigMsg1):
        setPropValue(setMode, targetValue, actualValue, msg1);
        break;
      case (BodyConfigMsg2):
        setPropValue(setMode, targetValue, actualValue, msg2);
        break;
      case (BodyConfigMsg3):
        setPropValue(setMode, targetValue, actualValue, msg3);
        break;
      case (BodyConfigMove):
        setPropValue(setMode, targetValue, actualValue, move);
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
