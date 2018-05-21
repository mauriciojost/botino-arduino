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
  void (*blackFace)();
  void (*whiteFace)();
  void (*arms)(int left, int right);
  void (*messageFunc)(int line, const char *msg, int size);
  void (*iosFunc)(char led, bool v);
  Buffer<MSG_MAX_LENGTH> *msgs[NRO_MSGS];
  Routine *routines[NRO_ROUTINES];

  bool isInitialized() {
    bool init = smilyFace != NULL && sadFace != NULL && normalFace != NULL && sleepyFace != NULL && blackFace != NULL && whiteFace != NULL && arms != NULL &&
                iosFunc != NULL && messageFunc != NULL;
    return init;
  }

  int getInt(char c) {
  	return ABSOL(c - '0') % 10;
  }

  bool getBool(char c) {
  	return c == 'y' || c == 'Y' || c == 't' || c == 'T';
  }

  void performPose(char c1, char c2, char c3) {
  	// Symbols of the poses documentation:
  	// - 'c' stands for a predefined alpha-numerical character (any), examples: a 6 x O Y
  	// - 'd' stands for a digit (only one, so a value between 0 and 9 inclusive), examples: 1 2 3 9 0
  	// - 'b' stands for a character representing a boolean value (Y/y meaning true, other character meaning false), examples Y y N n
  	// - '?' stands for any character that is not required for the pose (ignored)

    switch (c1) {

      // Wd? : WAIT -> wait d number of seconds
      case 'W':
        {
          int v = getInt(c2);
          log(CLASS_BODY, Debug, "Wait %d s", v);
          delay(v * 1000);
        }
        break;

      // Fc? : FACES -> show a given image in the LCD
      case 'F':
      	switch (c2) {
          // s -> smile
          case 's':
            log(CLASS_BODY, Debug, "Smile face");
            smilyFace();
            break;
          // S -> sad
          case 'S':
            log(CLASS_BODY, Debug, "Sad face");
            sadFace();
            break;
          // n -> normal
          case 'n':
            log(CLASS_BODY, Debug, "Normal face");
            normalFace();
            break;
          // l -> sleepy
          case 'l':
            log(CLASS_BODY, Debug, "Sleepy face");
            sleepyFace();
            break;
          // b -> black
          case 'b':
            log(CLASS_BODY, Debug, "Black face");
            blackFace();
            break;
          // w -> white
          case 'w':
            log(CLASS_BODY, Debug, "White face");
            whiteFace();
            break;
          default:
            log(CLASS_BODY, Debug, "Invalid face pose: %c%c%c", c1, c2, c3);
            break;
      	}
      	break;

      // Add : ARMS -> move both arms to a given position each (left, then right)
      case 'A':
        {
        	int l = getInt(c2);
        	int r = getInt(c3);
          log(CLASS_BODY, Debug, "Arms %d %d", l, r);
          arms(l, r);
        }
      	break;

      // Mc?: MESSAGES -> show certain messages in the LCD
      case 'M':
      	switch (c2) {
          // 0 -> show message 0
          case '0':
            log(CLASS_BODY, Debug, "Message 0");
            messageFunc(0, msgs[0]->getBuffer(), getInt(c3));
            break;
          // 1 -> show message 1
          case '1':
            log(CLASS_BODY, Debug, "Message 1");
            messageFunc(0, msgs[1]->getBuffer(), getInt(c3));
            break;
          // 2 -> show message 2
          case '2':
            log(CLASS_BODY, Debug, "Message 2");
            messageFunc(0, msgs[2]->getBuffer(), getInt(c3));
            break;
          // 3 -> show message 3
          case '3':
            log(CLASS_BODY, Debug, "Message 3");
            messageFunc(0, msgs[3]->getBuffer(), getInt(c3));
            break;
          // c -> show message containing current time
          case 'c': {
          	{
              log(CLASS_BODY, Debug, "Message clock");
              int h = GET_HOURS(timing.getCurrentTime());
              int m = GET_MINUTES(timing.getCurrentTime());
              Buffer<6> t("");
              t.fill("%02d:%02d", h, m);
              messageFunc(0, t.getBuffer(), getInt(c3));
          	}
            break;
          default:
            log(CLASS_BODY, Debug, "Invalid message pose: %c%c%c", c1, c2, c3);
            break;
          }
      	}
      	break;

      // Lcb: IO -> turn on/off a given IO device, such as LEDS or the FAN (true = ON)
      case 'L':
      	switch (c2) {
          // r -> turn on/off led red
          case 'r':
            {
              bool b = getBool(c3);
              log(CLASS_BODY, Debug, "Led red: %d", b);
              iosFunc('r', b);
              break;
            }
          // w -> turn on/off led white
          case 'w':
            {
              bool b = getBool(c3);
              log(CLASS_BODY, Debug, "Led white: %d", b);
              iosFunc('w', b);
              break;
            }
          // y -> turn on/off led yellow
          case 'y':
            {
              bool b = getBool(c3);
              log(CLASS_BODY, Debug, "Led yellow: %d", b);
              iosFunc('y', b);
              break;
            }
          // f -> turn on/off fan
          case 'f':
            {
              bool b = getBool(c3);
              log(CLASS_BODY, Debug, "Fan: %d", b);
              iosFunc('f', b);
              break;
            }
          default:
            log(CLASS_BODY, Debug, "Invalid IO pose: %c%c%c", c1, c2, c3);
            break;
      	}
      	break;

      default:

        switch (GET_POSE(c1, c2)) {

          // zz?: SWITCH OFF -> turn all power consuming components off
        	case GET_POSE('z', 'z'):
            iosFunc('r', false);
            iosFunc('w', false);
            iosFunc('y', false);
            iosFunc('f', false);
            blackFace();
            arms(0, 0);
            break;

          // DEFAULT
          default:
            log(CLASS_BODY, Debug, "Invalid pose: %c%c%c", c1, c2, c3);
            break;
        }
    }
  }

public:
  Body(const char *n) : timing(OnceEvery1Minute) {
    name = n;
    smilyFace = NULL;
    sadFace = NULL;
    normalFace = NULL;
    sleepyFace = NULL;
    blackFace = NULL;
    whiteFace = NULL;
    arms = NULL;
    messageFunc = NULL;
    iosFunc = NULL;
    for (int i = 0; i < NRO_MSGS; i++) {
      msgs[i] = new Buffer<MSG_MAX_LENGTH>("");
    }
    for (int i = 0; i < NRO_ROUTINES; i++) {
      routines[i] = new Routine();
      routines[i]->timingConf = 0L; // Never
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
  void setBlackFace(void (*f)()) {
    blackFace = f;
  }
  void setWhiteFace(void (*f)()) {
    whiteFace = f;
  }
  void setArms(void (*f)(int left, int right)) {
    arms = f;
  }
  void setMessageFunc(void (*f)(int line, const char *str, int size)) {
    messageFunc = f;
  }
  void setIosFunc(void (*f)(char led, bool v)) {
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
          performMove(i);
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
  	if (propIndex >= BodyConfigMove0 && propIndex < (NRO_ROUTINES + BodyConfigMove0)) {
      int i = (int)propIndex - (int)BodyConfigMove0;
      setPropValue(setMode, targetValue, actualValue, &routines[i]->move);
  	} else if (propIndex >= BodyConfigMsg0 && propIndex < (NRO_MSGS + BodyConfigMsg0)) {
      int i = (int)propIndex - (int)BodyConfigMsg0;
      setPropValue(setMode, targetValue, actualValue, msgs[i]);
    } else if (propIndex >= BodyConfigTime0 && propIndex < (NRO_ROUTINES + BodyConfigTime0)) {
      int i = (int)propIndex - (int)BodyConfigTime0;
      setPropLong(setMode, targetValue, actualValue, &routines[i]->timingConf);
      routines[i]->timing.setCustom(routines[i]->timingConf);
  	} else {
      switch (propIndex) {
        default:
          break;
      }
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

  void performMove(int routineIndex) {
    const char *s = routines[routineIndex % NRO_ROUTINES]->move.getBuffer();
    for (int i = 0; i < strlen(s); i += POSE_STR_LENGTH) {
      performPose(s[i + 0], s[i + 1], s[i + 2]);
    }
  }

};

#endif // BODY_INC
