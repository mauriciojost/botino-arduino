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

#define ON 1
#define OFF 0

#define POSI_VALUE(a, b) (int)(((int)(a)) * 256 + (b))

#define NRO_MSGS 4
#define NRO_MOVES 4

enum BodyConfigState {
  BodyConfigMsg0 = 0,         // message 0
  BodyConfigMsg1,             // message 1
  BodyConfigMsg2,             // message 2
  BodyConfigMsg3,             // message 3
  BodyConfigMove0,            // move 0
  BodyConfigMove1,            // move 1
  BodyConfigMove2,            // move 2
  BodyConfigMove3,            // move 3
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
  void (*ledFunc)(unsigned char led, unsigned char v);
  Buffer<MSG_MAX_LENGTH> **msgs;
  Buffer<MOVE_STR_LENGTH> **moves;
  long time;
  long cron;

  bool isInitialized() {
    return smilyFace != NULL &&
    		sadFace != NULL &&
				normalFace != NULL &&
				sleepyFace != NULL &&
				arms != NULL &&
        ledFunc != NULL &&
				messageFunc != NULL;
  }

  void doMovePosition(char c1, char c2) {
    switch (POSI_VALUE(c1, c2)) {
    	// faces
      case POSI_VALUE('f', 's'):
        log(CLASS_BODY, Debug, "Smile");
        smilyFace();
        break;
      case POSI_VALUE('f', 'S'):
        log(CLASS_BODY, Debug, "Sad");
        sadFace();
        break;
      case POSI_VALUE('f', 'n'):
        log(CLASS_BODY, Debug, "Normal");
        normalFace();
        break;
      case POSI_VALUE('f', 'l'):
        log(CLASS_BODY, Debug, "Sleepy");
        sleepyFace();
        break;
      // arms
      case POSI_VALUE('a', 'u'):
        log(CLASS_BODY, Debug, "Arms up");
        arms(ArmUp, ArmUp);
        break;
      case POSI_VALUE('a', 'r'):
        log(CLASS_BODY, Debug, "Arms down/up");
        arms(ArmDown, ArmUp);
        break;
      case POSI_VALUE('a', 'l'):
        log(CLASS_BODY, Debug, "Arms up/down");
        arms(ArmUp, ArmDown);
        break;
      case POSI_VALUE('a', 'd'):
        log(CLASS_BODY, Debug, "Arms down");
        arms(ArmDown, ArmDown);
        break;
      case POSI_VALUE('a', 'm'):
        log(CLASS_BODY, Debug, "Arms middle");
        arms(ArmMiddle, ArmMiddle);
        break;
      // messages
      case POSI_VALUE('m', '0'):
        log(CLASS_BODY, Debug, "Message 0");
        messageFunc(0, msgs[0]->getBuffer());
        break;
      case POSI_VALUE('m', '1'):
        log(CLASS_BODY, Debug, "Message 1");
        messageFunc(0, msgs[1]->getBuffer());
        break;
      case POSI_VALUE('m', '2'):
        log(CLASS_BODY, Debug, "Message 2");
        messageFunc(0, msgs[2]->getBuffer());
        break;
      case POSI_VALUE('m', '3'):
        log(CLASS_BODY, Debug, "Message 3");
        messageFunc(0, msgs[3]->getBuffer());
        break;
      // misc
      case POSI_VALUE('w', '1'):
        log(CLASS_BODY, Debug, "Wait 1s");
        delay(1000);
        break;
      case POSI_VALUE('w', '2'):
        log(CLASS_BODY, Debug, "Wait 2s");
        delay(2000);
        break;
      case POSI_VALUE('w', '3'):
        log(CLASS_BODY, Debug, "Wait 3s");
        delay(3000);
        break;
      // leds
      case POSI_VALUE('l', '0'):
        log(CLASS_BODY, Debug, "Led 0 on");
        ledFunc(0, ON);
        break;
      case POSI_VALUE('l', '1'):
        log(CLASS_BODY, Debug, "Led 1 on");
        ledFunc(1, ON);
        break;
      case POSI_VALUE('l', '2'):
        log(CLASS_BODY, Debug, "Led 2 on");
        ledFunc(2, ON);
        break;
      case POSI_VALUE('l', '3'):
        log(CLASS_BODY, Debug, "Led 3 on");
        ledFunc(3, ON);
        break;
      case POSI_VALUE('L', '0'):
        log(CLASS_BODY, Debug, "Led 0 off");
        ledFunc(0, OFF);
        break;
      case POSI_VALUE('L', '1'):
        log(CLASS_BODY, Debug, "Led 1 off");
        ledFunc(1, OFF);
        break;
      case POSI_VALUE('L', '2'):
        log(CLASS_BODY, Debug, "Led 2 off");
        ledFunc(2, OFF);
        break;
      case POSI_VALUE('L', '3'):
        log(CLASS_BODY, Debug, "Led 3 off");
        ledFunc(3, OFF);
        break;
      // default
      default:
        log(CLASS_BODY, Debug, "Invalid command");
        break;
    }
  }

  void doMove(const char* s) {
    log(CLASS_BODY, Debug, "Instr: %s", s);
    for (int i = 0; i < strlen(s); i+=2) {
      doMovePosition(s[i], s[i + 1]);
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
    ledFunc = NULL;
    time = 0L;
    cron = 0L;
    msgs = new Buffer<MSG_MAX_LENGTH>*[NRO_MSGS];
    for (int i = 0; i < NRO_MSGS; i++) {
      msgs[i] = new Buffer<MSG_MAX_LENGTH>("");
    }
    moves = new Buffer<MOVE_STR_LENGTH>*[NRO_MOVES];
    for (int i = 0; i < NRO_MOVES; i++) {
      moves[i] = new Buffer<MOVE_STR_LENGTH>("");
    }
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
  void setLedFunc(void (*f)(unsigned char led, unsigned char v)) {
    ledFunc = f;
  }

  void act() {
    if (!isInitialized()) {
      return;
    }
    if (freqConf.matches()) {
      log(CLASS_BODY, Debug, "Body up");
      doMove(moves[0]->getBuffer());
      doMove(moves[1]->getBuffer());
      doMove(moves[2]->getBuffer());
      doMove(moves[3]->getBuffer());
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (BodyConfigMsg0):
        return "ms0";
      case (BodyConfigMsg1):
        return "ms1";
      case (BodyConfigMsg2):
        return "ms2";
      case (BodyConfigMsg3):
        return "ms3";
      case (BodyConfigMove0):
        return "m0";
      case (BodyConfigMove1):
        return "m1";
      case (BodyConfigMove2):
        return "m2";
      case (BodyConfigMove3):
        return "m3";
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
        setPropValue(setMode, targetValue, actualValue, msgs[0]);
        break;
      case (BodyConfigMsg1):
        setPropValue(setMode, targetValue, actualValue, msgs[1]);
        break;
      case (BodyConfigMsg2):
        setPropValue(setMode, targetValue, actualValue, msgs[2]);
        break;
      case (BodyConfigMsg3):
        setPropValue(setMode, targetValue, actualValue, msgs[3]);
        break;
      case (BodyConfigMove0):
        setPropValue(setMode, targetValue, actualValue, moves[0]);
        break;
      case (BodyConfigMove1):
        setPropValue(setMode, targetValue, actualValue, moves[1]);
        break;
      case (BodyConfigMove2):
        setPropValue(setMode, targetValue, actualValue, moves[2]);
        break;
      case (BodyConfigMove3):
        setPropValue(setMode, targetValue, actualValue, moves[3]);
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
