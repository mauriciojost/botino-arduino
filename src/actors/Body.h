#ifndef BODY_INC
#define BODY_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_BODY "BO"
#define MSG_MAX_LENGTH 32
#define MAX_POSI_PER_MOVE 15 // maximum amount of positions per move
#define POSI_STR_LENGTH 3 // characters that represent a position / state within a move
#define MOVE_STR_LENGTH (POSI_STR_LENGTH * MAX_POSI_PER_MOVE)

#define ON 1
#define OFF 0

#define POSI_VALUE(a, b) (int)(((int)(a)) * 256 + (b))

#define NRO_MSGS 4
#define NRO_ROUTINES 4

enum BodyConfigState {
  BodyConfigMsg0 = 0,         // message 0
  BodyConfigMsg1,             // message 1
  BodyConfigMsg2,             // message 2
  BodyConfigMsg3,             // message 3
  BodyConfigMove0,            // move 0
  BodyConfigMove1,            // move 1
  BodyConfigMove2,            // move 2
  BodyConfigMove3,            // move 3
  BodyConfigTime0,            // time/cron of acting for move 0
  BodyConfigTime1,            // time/cron of acting for move 1
  BodyConfigTime2,            // time/cron of acting for move 2
  BodyConfigTime3,            // time/cron of acting for move 3
  BodyConfigStateDelimiter    // delimiter of the configuration states
};

enum ArmState { ArmUp = 0, ArmMiddle, ArmDown, ArmDelimiter };

class Routine {
public:
  Buffer<MOVE_STR_LENGTH> move;
  Timing timing;
  long timingConf;
};

class Body : public Actor {

private:
  const char *name;
  Timing timing;
  void (*smilyFace)();
  void (*sadFace)();
  void (*normalFace)();
  void (*sleepyFace)();
  void (*arms)(ArmState left, ArmState right);
  void (*messageFunc)(int line, const char *msg);
  void (*ledFunc)(unsigned char led, unsigned char v);
  Buffer<MSG_MAX_LENGTH> **msgs;
  Routine **routines;

  bool isInitialized() {
    return smilyFace != NULL &&
    		sadFace != NULL &&
				normalFace != NULL &&
				sleepyFace != NULL &&
				arms != NULL &&
        ledFunc != NULL &&
				messageFunc != NULL;
  }

  void doPosition(char c1, char c2) {
    switch (POSI_VALUE(c1, c2)) {

    	// FACES
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

      // ARMS
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
      case POSI_VALUE('a', 'w'):
        log(CLASS_BODY, Debug, "Arms waving");
        arms(ArmDown, ArmDown);
        arms(ArmUp, ArmUp);
        arms(ArmDown, ArmDown);
        break;
      case POSI_VALUE('a', 'm'):
        log(CLASS_BODY, Debug, "Arms middle");
        arms(ArmMiddle, ArmMiddle);
        break;

      // MESSAGES
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

      // WAITS
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

      // LEDS
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

      // DEFAULT
      default:
        log(CLASS_BODY, Debug, "Invalid pos: %c%c", c1, c2);
        break;
    }
  }

  void doMove(const char* s) {
    for (int i = 0; i < strlen(s); i+=POSI_STR_LENGTH) {
      doPosition(s[i], s[i + 1]);
    }
  }

public:

  Body(const char *n) : timing(OnceEvery1Minute) {
    name = n;
    smilyFace = NULL;
    sadFace = NULL;
    normalFace = NULL;
    sleepyFace = NULL;
    arms = NULL;
    messageFunc = NULL;
    ledFunc = NULL;
    msgs = new Buffer<MSG_MAX_LENGTH>*[NRO_MSGS];
    for (int i = 0; i < NRO_MSGS; i++) {
      msgs[i] = new Buffer<MSG_MAX_LENGTH>("");
    }
    routines = new Routine*[NRO_ROUTINES];
    for (int i = 0; i < NRO_ROUTINES; i++) {
    	routines[i] = new Routine();
      routines[i]->timingConf = 100000050L;
      routines[i]->timing.setCustom(routines[i]->timingConf);
      routines[i]->timing.setFrequency(Custom);
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
    for (int i = 0; i < NRO_MSGS; i++) {
    	while(routines[i]->timing.catchesUp(timing.getCurrentTime())) {
    		if (routines[i]->timing.matches()) {
          const char* move = routines[i]->move.getBuffer();
          log(CLASS_BODY, Debug, "Routine: %d %s", i, move);
          doMove(move);
    		}
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
      case (BodyConfigMove0):
        return "v0";
      case (BodyConfigMove1):
        return "v1";
      case (BodyConfigMove2):
        return "v2";
      case (BodyConfigMove3):
        return "v3";
      case (BodyConfigTime0):
        return "t0";
      case (BodyConfigTime1):
        return "t1";
      case (BodyConfigTime2):
        return "t2";
      case (BodyConfigTime3):
        return "t3";
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
        setPropValue(setMode, targetValue, actualValue, &routines[0]->move);
        break;
      case (BodyConfigMove1):
        setPropValue(setMode, targetValue, actualValue, &routines[1]->move);
        break;
      case (BodyConfigMove2):
        setPropValue(setMode, targetValue, actualValue, &routines[2]->move);
        break;
      case (BodyConfigMove3):
        setPropValue(setMode, targetValue, actualValue, &routines[3]->move);
        break;
      case (BodyConfigTime0):
        setPropLong(setMode, targetValue, actualValue, &routines[0]->timingConf);
        routines[0]->timing.setCustom(routines[0]->timingConf);
        break;
      case (BodyConfigTime1):
        setPropLong(setMode, targetValue, actualValue, &routines[1]->timingConf);
        routines[1]->timing.setCustom(routines[1]->timingConf);
        break;
      case (BodyConfigTime2):
        setPropLong(setMode, targetValue, actualValue, &routines[2]->timingConf);
        routines[2]->timing.setCustom(routines[2]->timingConf);
        break;
      case (BodyConfigTime3):
        setPropLong(setMode, targetValue, actualValue, &routines[3]->timingConf);
        routines[3]->timing.setCustom(routines[3]->timingConf);
        break;
      default:
        break;
    }
  }

  int getNroProps() { return BodyConfigStateDelimiter; }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() { return 0; }

  Timing *getFrequencyConfiguration() {
    return &timing;
  }
};

#endif // BODY_INC
