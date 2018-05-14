#ifndef BODY_INC
#define BODY_INC

#ifdef UNIT_TEST
void delay(int) {}
#endif // UNIT_TEST

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_BODY "BO"
#define MSG_MAX_LENGTH 32
#define MAX_POSES_PER_MOVE 15 // maximum amount of positions per move
#define POSE_STR_LENGTH 3     // characters that represent a position / state within a move
#define MOVE_STR_LENGTH (POSE_STR_LENGTH * MAX_POSES_PER_MOVE)

#define ON 1
#define OFF 0

#define GET_POSE(a, b) (int)(((int)(a)) * 256 + (b))

#define NRO_MSGS 4
#define NRO_ROUTINES 4

enum BodyConfigState {
  BodyConfigMsg0 = 0,      // message 0
  BodyConfigMsg1,          // message 1
  BodyConfigMsg2,          // message 2
  BodyConfigMsg3,          // message 3
  BodyConfigMove0,         // move 0
  BodyConfigMove1,         // move 1
  BodyConfigMove2,         // move 2
  BodyConfigMove3,         // move 3
  BodyConfigTime0,         // time/freq of acting for move 0
  BodyConfigTime1,         // time/freq of acting for move 1
  BodyConfigTime2,         // time/freq of acting for move 2
  BodyConfigTime3,         // time/freq of acting for move 3
  BodyConfigStateDelimiter // delimiter of the configuration states
};

enum ArmState { ArmUp = 0, ArmMiddle, ArmDown, ArmDelimiter };

class Routine {
public:
  Buffer<MOVE_STR_LENGTH> move;
  Timing timing;
  long timingConf;
};

/**
 * Body, representative of the Botino physical robot.
 *
 * The robot can perform a few configurable routines at a given moment / frequency in time.
 *
 * A routine is made of:
 * - a timing configuration (can be a frequency expression or a specific time of the day)
 * - a move (a sequence of poses)
 *
 * A move can contain many poses. A pose can be both arms up, both arms down, a given face (sad, smily) or even some
 * special conditions like illumination (red light) or control over the fan.
 *
 * Routines are configurable in both senses: timing and moves.
 */
class Body : public Actor {

private:
  const char *name;
  Timing timing;
  void (*smilyFace)();
  void (*sadFace)();
  void (*normalFace)();
  void (*sleepyFace)();
  void (*clearFace)();
  void (*arms)(ArmState left, ArmState right);
  void (*messageFunc)(int line, const char *msg, int size);
  void (*iosFunc)(unsigned char led, unsigned char v);
  Buffer<MSG_MAX_LENGTH> *msgs[NRO_MSGS];
  Routine *routines[NRO_ROUTINES];

  bool isInitialized() {
    bool init = smilyFace != NULL && sadFace != NULL && normalFace != NULL && sleepyFace != NULL && clearFace != NULL && arms != NULL &&
                iosFunc != NULL && messageFunc != NULL;
    return init;
  }

  int getInt(char c) {
  	return ABSOL(c - '0');
  }

  bool getBool(char c) {
  	return c == 'Y';
  }

  void performPose(char c1, char c2, char c3) {

    switch (c1) {

      // WAITS (Wd.)
      case 'W':
        {
          int v = getInt(c2);
          log(CLASS_BODY, Debug, "Wait %d s", v);
          delay(v * 1000);
        }
        break;

     // FACES (Fx.)
      case 'F':
      	switch (c2) {
          case 's':
            log(CLASS_BODY, Debug, "Smile");
            smilyFace();
            break;
          case 'S':
            log(CLASS_BODY, Debug, "Sad");
            sadFace();
            break;
          case 'n':
            log(CLASS_BODY, Debug, "Normal");
            normalFace();
            break;
          case 'l':
            log(CLASS_BODY, Debug, "Sleepy");
            sleepyFace();
            break;
          case 'c':
            log(CLASS_BODY, Debug, "Clear");
            clearFace();
            break;
      	}
      	break;

      // ARMS (Ax.)
      case 'A':

      	switch (c2) {
          case 'u':
            log(CLASS_BODY, Debug, "Arms up");
            arms(ArmUp, ArmUp);
            break;
          case 'r':
            log(CLASS_BODY, Debug, "Arms down/up");
            arms(ArmDown, ArmUp);
            break;
          case 'l':
            log(CLASS_BODY, Debug, "Arms up/down");
            arms(ArmUp, ArmDown);
            break;
          case 'd':
            log(CLASS_BODY, Debug, "Arms down");
            arms(ArmDown, ArmDown);
            break;
          case 'w':
            log(CLASS_BODY, Debug, "Arms waving");
            arms(ArmDown, ArmDown);
            arms(ArmUp, ArmUp);
            arms(ArmDown, ArmDown);
            break;
          case 'm':
            log(CLASS_BODY, Debug, "Arms middle");
            arms(ArmMiddle, ArmMiddle);
            break;
      	}
      	break;

     // MESSAGES (Mx.)
      case 'M':

      	switch (c2) {
          case '0':
            log(CLASS_BODY, Debug, "Message 0");
            messageFunc(0, msgs[0]->getBuffer(), getInt(c3));
            break;
          case '1':
            log(CLASS_BODY, Debug, "Message 1");
            messageFunc(0, msgs[1]->getBuffer(), getInt(c3));
            break;
          case '2':
            log(CLASS_BODY, Debug, "Message 2");
            messageFunc(0, msgs[2]->getBuffer(), getInt(c3));
            break;
          case '3':
            log(CLASS_BODY, Debug, "Message 3");
            messageFunc(0, msgs[3]->getBuffer(), getInt(c3));
            break;
          case 'c': {
            log(CLASS_BODY, Debug, "Message clock");
            int h = GET_HOURS(timing.getCurrentTime());
            int m = GET_MINUTES(timing.getCurrentTime());
            Buffer<6> t("");
            t.fill("%02d:%02d", h, m);
            messageFunc(0, t.getBuffer(), getInt(c3));
            break;
          }
      	}
      	break;

      // IO (LEDS) (Lx.)
      case 'L':
      	switch (c2) {
          case 'r':
            {
              bool b = !getBool(c3); // 0 -> ON
              log(CLASS_BODY, Debug, "Led red: %d", b);
              iosFunc('r', b);
              break;
            }
          case 'w':
            {
              bool b = !getBool(c3); // 0 -> ON
              log(CLASS_BODY, Debug, "Led white: %d", b);
              iosFunc('w', b);
              break;
            }
          case 'y':
            {
              bool b = !getBool(c3); // 0 -> ON
              log(CLASS_BODY, Debug, "Led yellow: %d", b);
              iosFunc('y', b);
              break;
            }
          case 'f':
            {
              bool b = getBool(c3); // 1 -> ON
              log(CLASS_BODY, Debug, "Fan: %d", b);
              iosFunc('f', b);
              break;
            }
      	}
      	break;

      default:

        switch (GET_POSE(c1, c2)) {
          // DEFAULT
          default:
            log(CLASS_BODY, Debug, "Invalid pose: %c%c%c", c1, c2, c3);
            break;
        }
    }
  }

  void performMove(const char *s) {
    for (int i = 0; i < strlen(s); i += POSE_STR_LENGTH) {
      performPose(s[i + 0], s[i + 1], s[i + 2]);
    }
  }

public:
  Body(const char *n) : timing(OnceEvery1Minute) {
    name = n;
    smilyFace = NULL;
    sadFace = NULL;
    normalFace = NULL;
    sleepyFace = NULL;
    clearFace = NULL;
    arms = NULL;
    messageFunc = NULL;
    iosFunc = NULL;
    for (int i = 0; i < NRO_MSGS; i++) {
      msgs[i] = new Buffer<MSG_MAX_LENGTH>("");
    }
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

  void setSmilyFace(void (*f)()) {
    smilyFace = f;
  }
  void setSadFace(void (*f)()) {
    sadFace = f;
  }
  void setNormalFace(void (*f)()) {
    normalFace = f;
  }
  void setSleepyFace(void (*f)()) {
    sleepyFace = f;
  }
  void setClearFace(void (*f)()) {
    clearFace = f;
  }
  void setArms(void (*f)(ArmState left, ArmState right)) {
    arms = f;
  }
  void setMessageFunc(void (*f)(int line, const char *str, int size)) {
    messageFunc = f;
  }
  void setIosFunc(void (*f)(unsigned char led, unsigned char v)) {
    iosFunc = f;
  }

  void act() {
    if (!isInitialized()) {
      log(CLASS_BODY, Warn, "No init!");
      return;
    }
    for (int i = 0; i < NRO_ROUTINES; i++) {
      while (routines[i]->timing.catchesUp(timing.getCurrentTime())) {
        if (routines[i]->timing.matches()) {
          const long timing = routines[i]->timingConf;
          const char *move = routines[i]->move.getBuffer();
          log(CLASS_BODY, Debug, "Rne %d: %ld %s", i, timing, move);
          performMove(move);
        }
      }
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (BodyConfigMsg0):
        return "msg0";
      case (BodyConfigMsg1):
        return "msg1";
      case (BodyConfigMsg2):
        return "msg2";
      case (BodyConfigMsg3):
        return "msg3";
      case (BodyConfigMove0):
        return "mv0";
      case (BodyConfigMove1):
        return "mv1";
      case (BodyConfigMove2):
        return "mv2";
      case (BodyConfigMove3):
        return "mv3";
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

  int getNroProps() {
    return BodyConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &timing;
  }
};

#endif // BODY_INC
