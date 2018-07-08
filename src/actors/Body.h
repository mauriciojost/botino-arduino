#ifndef BODY_INC
#define BODY_INC

/**
 * Body
 *
 * Representative of the Botino physical robot.
 *
 * The robot can perform a few configurable routines at a given moment / frequency in time.
 *
 * The most important concept is the routine, which is made of:
 * - a timing condition (can be a frequency expression or a specific time of the day)
 * - a move to perform once the timing condition matches (a sequence of poses)
 *
 * A move can contain many poses. A pose can be: both arms up, both arms down, a given face (sad, smily) or even some
 * special conditions like illumination (red light) or control over the fan.
 *
 * Routines are configurable in both senses: timing and moves.
 *
 */

#include <Hexer.h>
#include <actors/Images.h>
#include <actors/Messages.h>
#include <actors/Predictions.h>
#include <actors/Quotes.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Misc.h>
#include <main4ino/Value.h>

#define CLASS_BODY "BO"
#define MSG_MAX_LENGTH 32
#define MAX_POSES_PER_MOVE 6 // maximum amount of positions per move
#define POSE_STR_LENGTH 3    // characters that represent a position / state within a move
#define MOVE_STR_LENGTH (POSE_STR_LENGTH * MAX_POSES_PER_MOVE)

#define ON 1
#define OFF 0

#define GET_POSE(a, b) (int)(((int)(a)) * 256 + (b))

#define NRO_ROUTINES 8

#define ARM_SLOW_STEPS 100
#define ARM_NORMAL_STEPS 40
#define ARM_FAST_STEPS 20

#define IMG_SIZE_BYTES 16

enum BodyProps {
  BodyMove0Prop = 0, // string, move for the routine 0 as a list of consecutive 3 letter-code of poses (see the poses documentation)
  BodyMove1Prop,     // string, move for the routine 1 (same as above)
  BodyMove2Prop,     // string, move
  BodyMove3Prop,     // string, move
  BodyMove4Prop,     // string, move
  BodyMove5Prop,     // string, move
  BodyMove6Prop,     // string, move
  BodyMove7Prop,     // string, move
  BodyTime0Prop,     // time/freq of acting for the routine 0
  BodyTime1Prop,     // time/freq of acting for the routine 1
  BodyTime2Prop,     // time/freq
  BodyTime3Prop,     // time/freq
  BodyTime4Prop,     // time/freq
  BodyTime5Prop,     // time/freq
  BodyTime6Prop,     // time/freq
  BodyTime7Prop,     // time/freq
  BodyPropsDelimiter // delimiter of the configuration states
};

#define MOVE_DANCE0 "LwyFs.B09B90LwnFw.B09B90LwyFb.B55"
#define MOVE_DANCE1 "FfyLyyLwyFs.A50A05Fb.LryLwnA00A99Fw.LrnLwyA90A09Fb.LwnLyyA90A09Fw.Fs."
#define MOVE_DANCE2 "A87A78A87A78A12A21A12A21"

// Create images with:
// https://docs.google.com/spreadsheets/d/1jXa9mFxeiN_bUji_WiCPKO_gB6pxQUeQ5QxgoSINqdc/edit#gid=0
uint8_t IMG_CRAZY[] = {0x00, 0x00, 0x7F, 0x00, 0x41, 0x3E, 0x41, 0x22, 0x49, 0x2A, 0x41, 0x22, 0x7F, 0x3E, 0x00, 0x00};
uint8_t IMG_SMILY[] = {0x00, 0x00, 0x02, 0x40, 0x12, 0x48, 0x12, 0x48, 0x08, 0x10, 0x08, 0x10, 0x04, 0x20, 0x03, 0xC0};
uint8_t IMG_NORMAL[] = {0x00, 0x00, 0x02, 0x40, 0x02, 0x40, 0x00, 0x00, 0x01, 0x00, 0x01, 0x80, 0x04, 0x20, 0x03, 0xC0};
uint8_t IMG_ANGRY[] = {0x00, 0x00, 0x70, 0x0E, 0x49, 0x12, 0x55, 0x2A, 0x45, 0x22, 0x7D, 0xBE, 0x00, 0x00, 0x03, 0xC0};
uint8_t IMG_SAD[] = {0x06, 0x60, 0x08, 0x10, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x00, 0x00};
uint8_t IMG_SLEEPY[] = {0x00, 0x00, 0x00, 0x00, 0x12, 0x48, 0x0C, 0x30, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00};

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
  void (*arms)(int left, int right, int steps);
  void (*messageFunc)(int line, const char *msg, int size);
  void (*iosFunc)(char led, bool v);
  void((*lcdImgFunc)(char img, uint8_t bitmap[]));

  Quotes *quotes;
  Images *images;
  Messages *messages;
  Routine *routines[NRO_ROUTINES];

  bool isInitialized() {
    bool init = arms != NULL && iosFunc != NULL && messageFunc != NULL && lcdImgFunc != NULL && quotes != NULL && images != NULL &&
                messages != NULL;
    return init;
  }

  int getInt(char c) {
    return ABSOL(c - '0') % 10;
  }

  bool getBool(char c) {
    return c == 'y' || c == 'Y' || c == 't' || c == 'T' || c == '1';
  }

  void performPose(char c1, char c2, char c3) {

    /*

POSES (3 char codes)
--------------------

ARMS POSES: move both arms to a given position each (left, then right) (A=fast, B=normal, C=slow)
Codes:
  A00 : Move left and right arms to respective position 0 and 0 (both down) at high speed
  ...
  A90 : Move left and right arms to respective position 9 and 0 (left arm up) at high speed
  ...
  A99 : Move left and right arms to respective position 9 and 9 (both up) at high speed

  B99 : Move left and right arms to respective position 9 and 9 (both up) at normal speed

  C99 : Move left and right arms to respective position 9 and 9 (both up) at low speed


FACE POSES: show a given image in the LCD
Codes:
  Fw. : Face White
  Fb. : Face Black
  Fa. : Face Angry
  Fr. : Face cRazy
  Fl. : Face cLear
  Fs. : Face Smily
  FS. : Face Sad
  Fn. : Face Normal
  Fz. : Face Zleepy
  F0. : Face custom 0 (user provided)
  F1. : Face custom 1 (user provided)
  F2. : Face custom 2 (user provided)
  F3. : Face custom 3 (user provided)


IO POSES: turn on/off a given IO device, such as LEDS or the FAN (on = y, off = n)
Codes:
  Lry : turn on (y) the Red led
  Lrn : turn off (n) the Red led
  Lwy : turn on (y) led White
  Lyy : turn on (y) led Yellow
  Lfy : turn on (y) Fan


MESSAGE POSES: show a certain message in the LCD with a given font size
Codes:
  M01 : show message 0 with font size 1 (user provided)
  ...
  M32 : show message 3 with font size 2 (user provided)
  Mc4 : show message containing current time (with font size 4)
  Mp1 : show random future reading (with font size 1)
  Mq1 : show random quote (with font size 1)

COMPOSED POSES: dances and other predefined moves usable as poses
Codes:
  S00 : dance 0
  S01 : dance 1
  S02 : dance 2


WAIT POSES: wait a given number of seconds
Codes:
  W1. : wait 1 second
  ...
  W9. : wait 9 seconds


SPECIAL POSES
Codes:
  Zz. : turn all power consuming components off

*/

    switch (c1) {

      case 'W': {
        int v = getInt(c2);
        log(CLASS_BODY, Debug, "Wait %d s", v);
        delay(v * 1000);
      } break;

      case 'F':
        switch (c2) {
          case '0':
            lcdImgFunc('c', images->get(0)); // custom 0
            break;
          case '1':
            lcdImgFunc('c', images->get(1)); // custom 1
            break;
          case '2':
            lcdImgFunc('c', images->get(2)); // custom 2
            break;
          case '3':
            lcdImgFunc('c', images->get(3)); // custom 3
            break;
          case 'w':
            lcdImgFunc('w', NULL); // white
            break;
          case 'b':
            lcdImgFunc('b', NULL); // black
            break;
          case 'l':
            lcdImgFunc('l', NULL); // clear
            break;
          case 'r':
            lcdImgFunc('c', IMG_CRAZY); // crazy
            break;
          case 's':
            lcdImgFunc('c', IMG_SMILY); // smile
            break;
          case 'S':
            lcdImgFunc('c', IMG_SAD); // sad
            break;
          case 'n':
            lcdImgFunc('c', IMG_NORMAL); // normal
            break;
          case 'a':
            lcdImgFunc('c', IMG_ANGRY); // angry
            break;
          case 'z':
            lcdImgFunc('c', IMG_SLEEPY); // sleepy
            break;
          default:
            log(CLASS_BODY, Debug, "Face '%c'?", c2);
            break;
        }
        break;

      case 'A': {
        int l = getInt(c2);
        int r = getInt(c3);
        log(CLASS_BODY, Debug, "Armsf %d&%d", l, r);
        arms(l, r, ARM_FAST_STEPS);
      } break;
      case 'B': {
        int l = getInt(c2);
        int r = getInt(c3);
        log(CLASS_BODY, Debug, "Armsn %d&%d", l, r);
        arms(l, r, ARM_NORMAL_STEPS);
      } break;
      case 'C': {
        int l = getInt(c2);
        int r = getInt(c3);
        log(CLASS_BODY, Debug, "Armss %d&%d", l, r);
        arms(l, r, ARM_SLOW_STEPS);
      } break;
      case 'M':
        switch (c2) {
          case '0':
            log(CLASS_BODY, Debug, "Msg 0");
            messageFunc(0, messages->get(0), getInt(c3));
            break;
          case '1':
            log(CLASS_BODY, Debug, "Msg 1");
            messageFunc(0, messages->get(1), getInt(c3));
            break;
          case '2':
            log(CLASS_BODY, Debug, "Msg 2");
            messageFunc(0, messages->get(2), getInt(c3));
            break;
          case '3':
            log(CLASS_BODY, Debug, "Msg 3");
            messageFunc(0, messages->get(3), getInt(c3));
            break;
          case 'c': {
            log(CLASS_BODY, Debug, "Msg clock");
            int h = GET_HOURS(timing.getCurrentTime());
            int m = GET_MINUTES(timing.getCurrentTime());
            Buffer<6> t("");
            t.fill("%02d:%02d", h, m);
            messageFunc(0, t.getBuffer(), getInt(c3));
          } break;
          case 'q': {
            log(CLASS_BODY, Debug, "Msg quote");
            int i = random(NRO_QUOTES);
            messageFunc(0, quotes->getQuote(i), getInt(c3));
          } break;
          case 'p': {
            log(CLASS_BODY, Debug, "Msg prediction");
            Buffer<200> pr("");
            Predictions::getPrediction(&pr);
            messageFunc(0, pr.getBuffer(), getInt(c3));
          } break;
          default:
            log(CLASS_BODY, Debug, "Inv.M.pose:%c%c%c", c1, c2, c3);
            break;
        }
        break;

      case 'L':
        switch (c2) {
          case 'r': {
            bool b = getBool(c3);
            log(CLASS_BODY, Debug, "Led red: %d", b);
            iosFunc('r', b);
            break;
          }
          case 'w': {
            bool b = getBool(c3);
            log(CLASS_BODY, Debug, "Led white: %d", b);
            iosFunc('w', b);
            break;
          }
          case 'y': {
            bool b = getBool(c3);
            log(CLASS_BODY, Debug, "Led yellow: %d", b);
            iosFunc('y', b);
            break;
          }
          case 'f': {
            bool b = getBool(c3);
            log(CLASS_BODY, Debug, "Fan: %d", b);
            iosFunc('f', b);
            break;
          }
          default:
            log(CLASS_BODY, Debug, "Inv.IO.pose:%c%c%c", c1, c2, c3);
            break;
        }
        break;

      default:

        switch (GET_POSE(c1, c2)) {

          case GET_POSE('S', '0'):
            switch (c3) {
              case '0':
                performMove(MOVE_DANCE0);
                break;
              case '1':
                performMove(MOVE_DANCE1);
                break;
              case '2':
                performMove(MOVE_DANCE2);
                break;
              default:
                log(CLASS_BODY, Debug, "Inv.S.pose:%c%c%c", c1, c2, c3);
            }
            break;

          case GET_POSE('Z', 'z'):
            lcdImgFunc('b', NULL);
            lcdImgFunc('l', NULL);
            iosFunc('r', false);
            iosFunc('w', false);
            iosFunc('y', false);
            iosFunc('f', false);
            arms(0, 0, ARM_SLOW_STEPS);
            break;

          // DEFAULT
          default:
            log(CLASS_BODY, Debug, "Invalid pose: %c%c%c", c1, c2, c3);
            break;
        }
    }
  }

public:
  Body(const char *n) {
    name = n;
    arms = NULL;
    messageFunc = NULL;
    iosFunc = NULL;
    lcdImgFunc = NULL;
    quotes = NULL;
    images = NULL;
    messages = NULL;
    timing.setFrequency(OnceEvery1Minute);
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

  void setQuotes(Quotes *q) {
    quotes = q;
  }

  void setImages(Images *i) {
    images = i;
  }

  void setMessages(Messages *m) {
    messages = m;
  }

  void setLcdImgFunc(void (*f)(char img, uint8_t bitmap[])) {
    lcdImgFunc = f;
  }
  void setArmsFunc(void (*f)(int left, int right, int steps)) {
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
      case (BodyMove0Prop):
        return "mv0"; // move 0 (for routine 0)
      case (BodyMove1Prop):
        return "mv1";
      case (BodyMove2Prop):
        return "mv2";
      case (BodyMove3Prop):
        return "mv3";
      case (BodyMove4Prop):
        return "mv4";
      case (BodyMove5Prop):
        return "mv5";
      case (BodyMove6Prop):
        return "mv6";
      case (BodyMove7Prop):
        return "mv7";
      case (BodyTime0Prop):
        return "t0"; // timing 0 (for routine 0)
      case (BodyTime1Prop):
        return "t1";
      case (BodyTime2Prop):
        return "t2";
      case (BodyTime3Prop):
        return "t3";
      case (BodyTime4Prop):
        return "t4";
      case (BodyTime5Prop):
        return "t5";
      case (BodyTime6Prop):
        return "t6";
      case (BodyTime7Prop):
        return "t7";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    if (propIndex >= BodyMove0Prop && propIndex < (NRO_ROUTINES + BodyMove0Prop)) {
      int i = (int)propIndex - (int)BodyMove0Prop;
      setPropValue(m, targetValue, actualValue, &routines[i]->move);
    } else if (propIndex >= BodyTime0Prop && propIndex < (NRO_ROUTINES + BodyTime0Prop)) {
      int i = (int)propIndex - (int)BodyTime0Prop;
      setPropLong(m, targetValue, actualValue, &routines[i]->timingConf);
      routines[i]->timing.setCustom(routines[i]->timingConf);
    } else {
      switch (propIndex) {
        default:
          break;
      }
    }
  }

  int getNroProps() {
    return BodyPropsDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &timing;
  }

  const char *getMove(int moveIndex) {
    return routines[moveIndex % NRO_ROUTINES]->move.getBuffer();
  }

  void performMove(int moveIndex) {
    performMove(getMove(moveIndex));
  }

  void performMove(const char *move) {
    if (strlen(move) % POSE_STR_LENGTH != 0) {
      log(CLASS_BODY, Warn, "Invalid move: %s", move);
      return;
    }
    for (int i = 0; i < strlen(move); i += POSE_STR_LENGTH) {
      performPose(move[i + 0], move[i + 1], move[i + 2]);
    }
  }
};

#endif // BODY_INC
