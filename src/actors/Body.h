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

#include <string.h>
#include <Hexer.h>
#include <actors/Images.h>
#include <actors/Predictions.h>
#include <actors/Quotes.h>
#include <actors/Ifttt.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Misc.h>
#include <main4ino/Value.h>

#define CLASS_BODY "BO"

#define ON 1
#define OFF 0

#define GET_POSE(a, b) (int)(((int)(a)) * 256 + (b))

#define NRO_ROUTINES 8

#define ARM_SLOW_STEPS 100
#define ARM_NORMAL_STEPS 40
#define ARM_FAST_STEPS 20

#define IMG_SIZE_BYTES 16

enum BodyProps {
  BodyRoutine0Prop = 0, // string, routine for the routine 0
  BodyRoutine1Prop,     // string, routine for the routine 1
  BodyRoutine2Prop,     // string, routine
  BodyRoutine3Prop,     // string, routine
  BodyRoutine4Prop,     // string, routine
  BodyRoutine5Prop,     // string, routine
  BodyRoutine6Prop,     // string, routine
  BodyRoutine7Prop,     // string, routine
  BodyPropsDelimiter // delimiter of the configuration states
};

#define POSE_SEPARATOR '.'
#define TIMING_SEPARATOR ':'
#define TIMING_STR_LEN 9
#define TIMING_AND_SEPARATOR_STR_LEN (TIMING_STR_LEN + 1)

#define MOVE_STR_LENGTH (32 + TIMING_AND_SEPARATOR_STR_LEN)

#define MOVE_DANCE0 "LwyB09B90LwnB09B90LwyB55"
#define MOVE_DANCE1 "LfyLyyLwyA50A05LryLwnA00A99LrnLwyA90A09LwnLyyA90A09"
#define MOVE_DANCE2 "A87A78L?.A87A78L?.A12A21L?.A12A21L?."
#define MOVE_DANCE3 "Da/Da\\DauDan"
#define MOVE_DANCE4 "S4?"
#define MOVE_DANCE5 "S5?"
#define MOVE_DANCE6 "S6?"
#define MOVE_DANCE7 "S7?"

#define MOVE_DANCE_U "A87A78L?.A87A78L?.A87A78L?.A87A78L?."
#define MOVE_DANCE_n "A12A21L?.A12A21L?.A12A21L?.A12A21L?."
#define MOVE_DANCE_BACK_SLASH "A71A82L?.A71A82L?.A71A82L?.A71A82"
#define MOVE_DANCE_FORW_SLASH "A17A28L?.A17A28L?.A17A28L?.A17A28"

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
  Buffer<MOVE_STR_LENGTH> timingMove;
  Timing* timing;
  Routine(const char* n) {
  	timing = new Timing(n);
  }
  void load(const Value* v) {
    Buffer<MOVE_STR_LENGTH> aux;
    aux.load(v);
    load(aux.getBuffer());
  }
  void load(const char* str) {
  	if (str != NULL && strlen(str) > TIMING_AND_SEPARATOR_STR_LEN) {
  		timingMove.fill(str);
  		timingMove.getUnsafeBuffer()[TIMING_STR_LEN] = 0;
  		timing->setFrek(atol(timingMove.getBuffer()));
  		timingMove.fill(str);
      log(CLASS_BODY, Debug, "Routine built: '%s'/'%ld'", getMove(), timing->getFrek());
  	} else {
      log(CLASS_BODY, Warn, "Invalid routine");
  	}
  }
  const char* getMove() {
    return timingMove.getBuffer() + TIMING_AND_SEPARATOR_STR_LEN;
  }
};

class Body : public Actor {

private:
  const char *name;
  Metadata* md;
  void (*arms)(int left, int right, int steps);
  void (*messageFunc)(int line, const char *msg, int size);
  void (*iosFunc)(char led, bool v);
  void((*lcdImgFunc)(char img, uint8_t bitmap[]));

  Quotes *quotes;
  Images *images;
  Ifttt *ifttt;
  Routine *routines[NRO_ROUTINES];

  bool isInitialized() {
    bool init = arms != NULL && iosFunc != NULL && messageFunc != NULL && lcdImgFunc != NULL && quotes != NULL && images != NULL && ifttt != NULL;
    return init;
  }

  int getInt(char c) {
    return ABSOL(c - '0');
  }

  bool getBool(char c) {
    return c == 'y' || c == 'Y' || c == 't' || c == 'T' || c == '1';
  }

  int poseStrLen(const char* p) {
  	if (p == NULL) { // no string
  		return -1;
  	} else {
      const char* f = strchr((p), POSE_SEPARATOR);
      if (f == NULL) { // no separator found
        return strlen(p);
      } else { // separator found
        return (int)(f - p);
      }
    }
  }

  /**
   * Perform a given pose
   *
   * Returns the next pose in the string or NULL if no more poses to perform.
   */
  const char* performPose(const char* pose) {

    /*

POSES (X-char codes separated by separator)
--------------------

### 1 LETTER CODE POSES

Codes:
  Z. : turn all power consuming components off


### 2 LETTER CODE POSES

WAIT POSES: wait a given number of seconds
Codes:
  W1. : wait 1 second
  ...
  W9. : wait 9 seconds

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
  F_. : Face dimmed
  F-. : Face bright
  F0. : Face custom 0 (user provided)
  F1. : Face custom 1 (user provided)
  F2. : Face custom 2 (user provided)
  F3. : Face custom 3 (user provided)

IFTTT EVENTS: trigger an ifttt event (given the configuration of the ifttt module)
Codes:
  Ix. : trigger event 'x'


### 3 LETTER CODE POSES

ARMS POSES: move both arms to a given position each (left, then right) (A=fast, B=normal, C=slow)
Codes:
  A00. : Move left and right arms to respective position 0 and 0 (both down) at high speed
  ...
  A90. : Move left and right arms to respective position 9 and 0 (left arm up) at high speed
  ...
  A99. : Move left and right arms to respective position 9 and 9 (both up) at high speed

  B99. : Move left and right arms to respective position 9 and 9 (both up) at normal speed

  C99. : Move left and right arms to respective position 9 and 9 (both up) at low speed

IO POSES: turn on/off a given IO device, such as LEDS or the FAN (on = y, off = n)
Codes:
  Lry. : turn on (y) the Red led
  Lrn. : turn off (n) the Red led
  Lwy. : turn on (y) led White
  Lyy. : turn on (y) led Yellow
  L?.. : turn randomly all leds
  Lfy. : turn on (y) Fan

COMPOSED POSES: dances and other predefined moves usable as poses
Codes:
  Dan. : dance n
  Dau. : dance u
  Da\. : dance \
  Da/. : dance /

  Da0. : dance 0
  Da1. : dance 1
  Da2. : dance 2
  Da3. : dance 3

MESSAGE POSES: show a certain message in the LCD with a given font size
Codes:
  Mc4 : show message containing current time (with font size 4)
  Mk3 : show message containing current date-time (with font size 3)
  Mp1 : show random future reading (with font size 1)
  Mq1 : show random quote (with font size 1)


### N>3 LETTER CODE POSES

MESSAGE POSES: show a certain message in the LCD with a given font size
Codes:
  M1HELLO. : show message HELLO with font size 1 (user provided)

*/

  if (poseStrLen(pose) == 0) { // 0 chars poses
  	return NULL;
  } else if (poseStrLen(pose) == 1) { // 1 chars poses
      char c1 = pose[0];
      if (c1 == 'Z') {
        lcdImgFunc('b', NULL);
        lcdImgFunc('l', NULL);
        iosFunc('r', false);
        iosFunc('w', false);
        iosFunc('y', false);
        iosFunc('f', false);
        arms(0, 0, ARM_FAST_STEPS);
      } else {
        log(CLASS_BODY, Debug, "Ignoring 1-letter-code pose %s", pose);
      }
      return pose + 1 + 1;
  	} else if (poseStrLen(pose) == 2) { // 2 chars poses
      char c1 = pose[0];
      char c2 = pose[1];

      if (c1 == 'W') { // WAIT
        int v = getInt(c2);
        log(CLASS_BODY, Debug, "Wait %d s", v);
        delay(v * 1000);
      } else if (c1 == 'F') { // FACES
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
          case '_':
            lcdImgFunc('_', NULL); // dim
            break;
          case '-':
            lcdImgFunc('-', NULL); // bright
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
      } else if (c1 == 'I') { // IFTTT
        int i = getInt(c2);
        log(CLASS_BODY, Debug, "Ifttt %d", i);
        ifttt->triggerEvent(i);
      } else {
        log(CLASS_BODY, Debug, "Ignoring 2-letter-code pose %s", pose);
      }
      return pose + 2 + 1;

  	} else if (poseStrLen(pose) == 3) { // 3 chars poses
      char c1 = pose[0];
      char c2 = pose[1];
      char c3 = pose[2];

      if (c1 == 'A') { // ARMS FAST
        int l = getInt(c2);
        int r = getInt(c3);
        log(CLASS_BODY, Debug, "Armsf %d&%d", l, r);
        arms(l, r, ARM_FAST_STEPS);
      } else if (c1 == 'B') { // ARMS MEDIUM
        int l = getInt(c2);
        int r = getInt(c3);
        log(CLASS_BODY, Debug, "Armsn %d&%d", l, r);
        arms(l, r, ARM_NORMAL_STEPS);
      } else if (c1 == 'C') { // ARMS SLOW
        int l = getInt(c2);
        int r = getInt(c3);
        log(CLASS_BODY, Debug, "Armss %d&%d", l, r);
        arms(l, r, ARM_SLOW_STEPS);
      } else if (c1 == 'L') { // IO (LEDS / FAN)
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
          case '?': {
            iosFunc('r', random(2) == 0);
            iosFunc('w', random(2) == 0);
            iosFunc('y', random(2) == 0);
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

      } else if (c1 == 'D') { // DANCE
        switch (c2) {
          case '0':
            performMove(MOVE_DANCE0);
            break;
          case '1':
            performMove(MOVE_DANCE1);
            break;
          case '2':
            performMove(MOVE_DANCE2);
            break;
          case '3':
            performMove(MOVE_DANCE3);
            break;
          case '4':
            performMove(MOVE_DANCE4);
            break;
          case '5':
            performMove(MOVE_DANCE5);
            break;
          case '6':
            performMove(MOVE_DANCE6);
            break;
          case '7':
            performMove(MOVE_DANCE7);
            break;
          case 'n':
            performMove(MOVE_DANCE_n);
            break;
          case 'u':
            performMove(MOVE_DANCE_U);
            break;
          case '\\':
            performMove(MOVE_DANCE_BACK_SLASH);
            break;
          case '/':
            performMove(MOVE_DANCE_FORW_SLASH);
            break;
          default:
            log(CLASS_BODY, Debug, "Inv.S.pose:%c%c%c", c1, c2, c3);
        }

      } else if (c1 == 'M') { // MESSAGES
        switch (c2) {
          case 'c': {
            log(CLASS_BODY, Debug, "Msg clock");
            int h = GET_HOURS(getTiming()->getCurrentTime());
            int m = GET_MINUTES(getTiming()->getCurrentTime());
            Buffer<6> t("");
            t.fill("%02d:%02d", h, m);
            messageFunc(0, t.getBuffer(), getInt(c3));
          } break;
          case 'k': {
            log(CLASS_BODY, Debug, "Msg date");
            long t = getTiming()->getCurrentTime();
            Buffer<18> b("");
            b.fill("%4d-%02d-%02d\n%02d:%02d", GET_YEARS(t), GET_MONTHS(t), GET_DAYS(t), GET_HOURS(t), GET_MINUTES(t));
            messageFunc(0, b.getBuffer(), getInt(c3));
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
            log(CLASS_BODY, Debug, "Invalid message %s", pose);
            break;
        }
      } else {
        log(CLASS_BODY, Warn, "Ignoring 3-letter-code pose %s", pose);
      }
      return pose + 3 + 1;
  	} else if (poseStrLen(pose) > 3) { // N chars poses

      char c1 = pose[0];
      char c2 = pose[1];
      char c3 = pose[2];

      if (c1 == 'M') { // MESSAGES
        int size = getInt(c2);
        Buffer<MOVE_STR_LENGTH> msg(pose + 2);
        msg.replace('.', 0);
        log(CLASS_BODY, Debug, "Msg '%s'", msg.getBuffer());
        messageFunc(0, msg.getBuffer(), size);
      } else {
        log(CLASS_BODY, Warn, "Ignoring N-letter-code pose %s", pose);
      }
      return pose + poseStrLen(pose) + 1;
  	} else {
      return NULL;
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
    ifttt = NULL;
    md = new Metadata(n);
    md->getTiming()->setFrek(201010101);
    for (int i = 0; i < NRO_ROUTINES; i++) {
      routines[i] = new Routine("mv");
      routines[i]->load("000000000:Z.");
    }

    // Overwrite last to setup clock
    routines[NRO_ROUTINES - 1]->load("201010160:Mc4"); // once every 1 minutes
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

  void setIfttt(Ifttt *i) {
    ifttt = i;
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
      while (routines[i]->timing->catchesUp(getTiming()->getCurrentTime())) {
        if (routines[i]->timing->matches()) {
          const long timing = routines[i]->timing->getFrek();
          log(CLASS_BODY, Debug, "Rne %d: %ld %s", i, timing, getMove(i));
          performMove(i);
        }
      }
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (BodyRoutine0Prop):
        return "r0"; // routine 0
      case (BodyRoutine1Prop):
        return "r1";
      case (BodyRoutine2Prop):
        return "r2";
      case (BodyRoutine3Prop):
        return "r3";
      case (BodyRoutine4Prop):
        return "r4";
      case (BodyRoutine5Prop):
        return "r5";
      case (BodyRoutine6Prop):
        return "r6";
      case (BodyRoutine7Prop):
        return "r7";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    if (propIndex >= BodyRoutine0Prop && propIndex < (NRO_ROUTINES + BodyRoutine0Prop)) {
      int i = (int)propIndex - (int)BodyRoutine0Prop;
      if (m == SetCustomValue) {
        routines[i]->load(targetValue);
      }
      if (actualValue != NULL) {
        actualValue->load(&routines[i]->timingMove);
      }
    }
    if (m != GetValue) {
    	getMetadata()->changed();
    }
  }

  int getNroProps() {
    return BodyPropsDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Metadata *getMetadata() {
    return md;
  }

  const char *getMove(int moveIndex) {
    return routines[POSIT(moveIndex % NRO_ROUTINES)]->getMove();
  }

  void performMove(int moveIndex) {
    performMove(getMove(moveIndex));
  }

  void performMove(const char *move) {
    int advance = 0;
    const char* p = move;
    while((p = performPose(p)) != NULL){ }
  }
};

#endif // BODY_INC
