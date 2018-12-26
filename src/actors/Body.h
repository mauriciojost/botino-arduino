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

COMPOSED POSES: dances and other predefined moves usable as poses
Codes:
  Dn. : dance n
  Du. : dance u
  D\. : dance \
  D/. : dance /

  D0. : dance 0
  D1. : dance 1
  D2. : dance 2
  D3. : dance 3

IFTTT EVENTS: trigger an ifttt event (given the configuration of the ifttt module)
Codes:
  Ix. : trigger event 'x'

IO POSES: turn on/off a given IO device, such as LEDS or the FAN (on = y, off = n)
Codes:
  L?.. : turn randomly all leds

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
  Lrt. : toggle (t) the Red led
  Lwy. : turn on (y) led White
  Lyy. : turn on (y) led Yellow
  Lyt. : toggle (t) led Yellow
  L?.. : turn randomly all leds
  Lfy. : turn on (y) Fan

MESSAGE POSES: show a certain message in the LCD with a given font size
Codes:
  Mc4. : show message containing current time (with font size 4)
  Mk3. : show message containing current date-time (with font size 3)
  Mp1. : show random future reading (with font size 1)
  Mq1. : show random quote (with font size 1)


### N>3 LETTER CODE POSES

MESSAGE POSES: show a certain message in the LCD with a given font size
Codes:
  M1HELLO. : show message HELLO with font size 1 (user provided)

NOTIFICATION POSES: show a certain notification in the LCD (requires user's ACK before removal)
Codes:
  NHELLO. : show notification HELLO

IFTTT EVENTS: trigger an ifttt event (by its name)
Codes:
  Iname. : trigger event 'name'


*/

#include <Hexer.h>
#include <actors/Ifttt.h>
#include <actors/Images.h>
#include <actors/Notifier.h>
#include <actors/Predictions.h>
#include <actors/Quotes.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Misc.h>
#include <main4ino/Timing.h>
#include <string.h>
#include <Io.h>

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
  BodyPropsDelimiter    // delimiter of the configuration states
};

#define POSE_SEPARATOR '.'
#define TIMING_SEPARATOR ':'
#define TIMING_AND_SEPARATOR_STR_LEN (TIMING_STR_LEN + 1)

#define MOVE_STR_LENGTH (32 + TIMING_AND_SEPARATOR_STR_LEN)

#define MOVE_DANCE0 "Lwy.B09.B90.Lwn.B09.B90.Lwy.B55."
#define MOVE_DANCE1 "Lfy.Lyy.Lwy.A50.A05.Lry.Lwn.A00.A99.Lrn.Lwy.A90.A09.Lwn.Lyy.A90.A09."
#define MOVE_DANCE2 "A87.A78.L?.A87.A78.L?.A12.A21.L?.A12.A21.L?."
#define MOVE_DANCE3 "Da.D\\.Du.Dn."
#define MOVE_DANCE4 "S4?"
#define MOVE_DANCE5 "S5?"
#define MOVE_DANCE6 "S6?"
#define MOVE_DANCE7 "S7?"

#define MOVE_DANCE_U "A87.A78.L?.A87.A78.L?.A87.A78.L?.A87.A78.L?."
#define MOVE_DANCE_n "A12.A21.L?.A12.A21.L?.A12.A21.L?.A12.A21.L?."
#define MOVE_DANCE_BACK_SLASH "A71.A82.L?.A71.A82.L?.A71.A82.L?.A71.A82."
#define MOVE_DANCE_FORW_SLASH "A17.A28.L?.A17.A28.L?.A17.A28.L?.A17.A28."

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
  Buffer *timingMove;
  Timing *timing;
  Routine(const char *n) {
    timingMove = new Buffer(MOVE_STR_LENGTH);
    timing = new Timing(n);
  }
  void load(const Value *v) {
    Buffer aux(MOVE_STR_LENGTH);
    aux.load(v);
    load(aux.getBuffer());
  }
  void load(const char *str) {
    if (str == NULL) {
      return;
    }
    timingMove->fill(str);
    timingMove->replace(':', 0);
    timing->setFreq(timingMove->getBuffer());

    timingMove->fill(str);
    log(CLASS_BODY, Debug, "Routine built: '%s'/'%s'", getMove(), timing->getFreq());
  }
  const char *getMove() {
    const char *m = timingMove->since(':');
    if (m != NULL) {
      return m;
    } else {
      return "?";
    }
  }
  Timing *getTiming() {
    return timing;
  }
};

class Body : public Actor {

private:
  const char *name;
  Metadata *md;
  void (*arms)(int left, int right, int steps);
  Notifier *notifier;
  void (*iosFunc)(char led, int v);
  Quotes *quotes;
  Images *images;
  Ifttt *ifttt;
  Routine *routines[NRO_ROUTINES];

  bool isInitialized() {
    bool init =
        arms != NULL && iosFunc != NULL && notifier != NULL && quotes != NULL && images != NULL && ifttt != NULL;
    return init;
  }

  int getInt(char c) {
    // '0' -> 0, ..., '9' -> 9
    return ABSOL(c - '0');
  }

  int getIosState(char c) {
  	if (c == 'n' || c == 'N' || c == '0') {
  		return IO_OFF;
  	} else if (c == 'y' || c == 'Y' || c == '1') {
  		return IO_ON;
  	} else if (c == 't' || c == 'T') {
  		return IO_TOGGLE;
  	} else {
  		return IO_OFF;
  	}
  }

  int poseStrLen(const char *p) {
    if (p == NULL) { // no string
      return -1;
    } else {
      const char *f = strchr((p), POSE_SEPARATOR);
      if (f == NULL) { // no separator found
        return -1;
      } else { // separator found
        return (int)(f - p);
      }
    }
  }

  void handle1CharPoses(char c1, const char *pose) {
    if (c1 == 'Z') {
      notifier->lcdImg('b', NULL);
      notifier->lcdImg('l', NULL);
      iosFunc('r', IO_OFF);
      iosFunc('w', IO_OFF);
      iosFunc('y', IO_OFF);
      iosFunc('f', IO_OFF);
      arms(0, 0, ARM_FAST_STEPS);
    } else {
      log(CLASS_BODY, Debug, "Ignoring 1-letter-code pose %s", pose);
    }
  }

  void handle2CharPoses(char c1, char c2, const char *pose) {
    if (c1 == 'W') {
      // WAIT
      int v = getInt(c2);
      log(CLASS_BODY, Debug, "Wait %d s", v);
      delay(v * 1000);
    } else if (c1 == 'F') {
      // FACES
      switch (c2) {
        case '0':
          notifier->lcdImg('c', images->get(0)); // custom 0
          break;
        case '1':
          notifier->lcdImg('c', images->get(1)); // custom 1
          break;
        case '2':
          notifier->lcdImg('c', images->get(2)); // custom 2
          break;
        case '3':
          notifier->lcdImg('c', images->get(3)); // custom 3
          break;
        case '_':
          notifier->lcdImg('_', NULL); // dim
          break;
        case '-':
          notifier->lcdImg('-', NULL); // bright
          break;
        case 'w':
          notifier->lcdImg('w', NULL); // white
          break;
        case 'b':
          notifier->lcdImg('b', NULL); // black
          break;
        case 'l':
          notifier->lcdImg('l', NULL); // clear
          break;
        case 'r':
          notifier->lcdImg('c', IMG_CRAZY); // crazy
          break;
        case 's':
          notifier->lcdImg('c', IMG_SMILY); // smile
          break;
        case 'S':
          notifier->lcdImg('c', IMG_SAD); // sad
          break;
        case 'n':
          notifier->lcdImg('c', IMG_NORMAL); // normal
          break;
        case 'a':
          notifier->lcdImg('c', IMG_ANGRY); // angry
          break;
        case 'z':
          notifier->lcdImg('c', IMG_SLEEPY); // sleepy
          break;
        default:
          log(CLASS_BODY, Debug, "Face '%c'?", c2);
          break;
      }
    } else if (c1 == 'I') {
      // IFTTT
      int i = getInt(c2);
      log(CLASS_BODY, Debug, "Ifttt %d", i);
      ifttt->triggerEvent(i);
    } else if (c1 == 'L') {
      // IO (LEDS / FAN)
      switch (c2) {
        case '?': {
          iosFunc('r', random(2));
          iosFunc('w', random(2));
          iosFunc('y', random(2));
          break;
        }
        default:
          log(CLASS_BODY, Debug, "Inv.IO.pose:%c%c", c1, c2);
          break;
      }
    } else if (c1 == 'D') {
      // DANCE
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
          log(CLASS_BODY, Debug, "Inv. dance:%c%c", c1, c2);
      }
    } else {
      log(CLASS_BODY, Debug, "Ignoring 2-letter-code pose %s", pose);
    }
  }

  void handle3CharPoses(char c1, char c2, char c3, const char *pose) {
    if (c1 == 'A') {
      // ARMS FAST
      int l = getInt(c2);
      int r = getInt(c3);
      log(CLASS_BODY, Debug, "Armsf %d&%d", l, r);
      arms(l, r, ARM_FAST_STEPS);
    } else if (c1 == 'B') {
      // ARMS MEDIUM
      int l = getInt(c2);
      int r = getInt(c3);
      log(CLASS_BODY, Debug, "Armsn %d&%d", l, r);
      arms(l, r, ARM_NORMAL_STEPS);
    } else if (c1 == 'C') {
      // ARMS SLOW
      int l = getInt(c2);
      int r = getInt(c3);
      log(CLASS_BODY, Debug, "Armss %d&%d", l, r);
      arms(l, r, ARM_SLOW_STEPS);
    } else if (c1 == 'L') {
      // IO (LEDS / FAN)
      switch (c2) {
        case 'r': {
          int b = getIosState(c3);
          log(CLASS_BODY, Debug, "Led red: %d", b);
          iosFunc('r', b);
          break;
        }
        case 'w': {
          int b = getIosState(c3);
          log(CLASS_BODY, Debug, "Led white: %d", b);
          iosFunc('w', b);
          break;
        }
        case 'y': {
          int b = getIosState(c3);
          log(CLASS_BODY, Debug, "Led yellow: %d", b);
          iosFunc('y', b);
          break;
        }
        case 'f': {
          int b = getIosState(c3);
          log(CLASS_BODY, Debug, "Fan: %d", b);
          iosFunc('f', b);
          break;
        }
        default:
          log(CLASS_BODY, Debug, "Inv.IO.pose:%c%c%c", c1, c2, c3);
          break;
      }
    } else if (c1 == 'M') {
      // MESSAGES
      switch (c2) {
        case 'c': {
          log(CLASS_BODY, Debug, "Msg clock");
          int h = GET_HOURS(getTiming()->getCurrentTime());
          int m = GET_MINUTES(getTiming()->getCurrentTime());
          Buffer t(6, "");
          t.fill("%02d:%02d", h, m);
          notifier->message(0, getInt(c3), t.getBuffer());
        } break;
        case 'k': {
          log(CLASS_BODY, Debug, "Msg date");
          long t = getTiming()->getCurrentTime();
          Buffer b(18, "");
          b.fill("%4d-%02d-%02d\n%02d:%02d", GET_YEARS(t), GET_MONTHS(t), GET_DAYS(t), GET_HOURS(t), GET_MINUTES(t));
          notifier->message(0, getInt(c3), b.getBuffer());
        } break;
        case 'q': {
          log(CLASS_BODY, Debug, "Msg quote");
          int i = random(NRO_QUOTES);
          notifier->message(0, getInt(c3), quotes->getQuote(i));
        } break;
        case 'p': {
          log(CLASS_BODY, Debug, "Msg prediction");
          Buffer pr(200, "");
          Predictions::getPrediction(&pr);
          notifier->message(0, getInt(c3), pr.getBuffer());
        } break;
        default:
          log(CLASS_BODY, Debug, "Invalid message %s", pose);
          break;
      }
    } else {
      log(CLASS_BODY, Warn, "Ignoring 3-letter-code pose %s", pose);
    }
  }

  void handleNCharPoses(char c1, char c2, const char *pose) {
    if (c1 == 'M') {
      // MESSAGES
      int size = getInt(c2);
      Buffer msg(MOVE_STR_LENGTH, pose + 2);
      msg.replace('.', 0);
      log(CLASS_BODY, Debug, "Msg '%s'", msg.getBuffer());
      notifier->message(0, size, msg.getBuffer());
    } else if (c1 == 'N') {
      // NOTIFICATION
      Buffer msg(MOVE_STR_LENGTH, pose + 1);
      msg.replace('.', 0);
      log(CLASS_BODY, Debug, "Not '%s'", msg.getBuffer());
      notifier->notification(msg.getBuffer());
    } else if (c1 == 'I') {
      // IFTTT
      Buffer evt(MOVE_STR_LENGTH, pose + 1);
      evt.replace('.', 0);
      log(CLASS_BODY, Debug, "Event '%s'", evt.getBuffer());
      ifttt->triggerEvent(evt.getBuffer());
    } else {
      log(CLASS_BODY, Warn, "Ignoring N-letter-code pose %s", pose);
    }
  }

public:
  /**
   * Perform a given pose
   *
   * Returns the next pose in the string or NULL if no more poses to perform.
   */
  const char *performPose(const char *pose) {

    int poseLen = poseStrLen(pose);

    if (poseLen == -1) { // invalid number of chars poses
      return NULL;
    } else if (poseLen == 0) { // 0 chars poses
      return NULL;
    } else if (poseLen == 1) { // 1 chars poses
      char c1 = pose[0];
      handle1CharPoses(c1, pose);
      return pose + 1 + 1;
    } else if (poseLen == 2) { // 2 chars poses
      char c1 = pose[0];
      char c2 = pose[1];
      handle2CharPoses(c1, c2, pose);
      return pose + 2 + 1;
    } else if (poseLen == 3) { // 3 chars poses
      char c1 = pose[0];
      char c2 = pose[1];
      char c3 = pose[2];
      handle3CharPoses(c1, c2, c3, pose);
      return pose + 3 + 1;
    } else if (poseLen > 3) { // N chars poses
      char c1 = pose[0];
      char c2 = pose[1];
      handleNCharPoses(c1, c2, pose);
      return pose + poseLen + 1;
    } else {
      return NULL;
    }
  }

  Body(const char *n) {
    name = n;
    arms = NULL;
    iosFunc = NULL;
    quotes = NULL;
    images = NULL;
    ifttt = NULL;
    notifier = NULL;
    md = new Metadata(n);
    md->getTiming()->setFreq("every1m");
    for (int i = 0; i < NRO_ROUTINES; i++) {
      routines[i] = new Routine("r");
      routines[i]->load("never:Z.");
    }

    // Overwrite last to setup clock
    routines[NRO_ROUTINES - 1]->load("every2m:Mc3."); // once every 1 minutes
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

  void setNotifier(Notifier *n) {
    notifier = n;
  }

  void setIfttt(Ifttt *i) {
    ifttt = i;
  }

  void setArmsFunc(void (*f)(int left, int right, int steps)) {
    arms = f;
  }

  void setIosFunc(void (*f)(char led, int v)) {
    iosFunc = f;
  }

  void act() {
    if (!isInitialized()) {
      log(CLASS_BODY, Warn, "No init!");
      return;
    }
    for (int i = 0; i < NRO_ROUTINES; i++) {
      routines[i]->timing->setCurrentTime(getTiming()->getCurrentTime()); // align with Body's time
      if (routines[i]->timing->matches()) {
        const char *timing = routines[i]->timing->getFreq();
        log(CLASS_BODY, Debug, "Rne %d: %s %s", i, timing, getMove(i));
        performMove(i);
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
        actualValue->load(routines[i]->timingMove);
      }
    }
    if (m != GetValue) {
      getMetadata()->changed();
    }
  }

  int getNroProps() {
    return BodyPropsDelimiter;
  }

  void getInfo(int infoIndex, Buffer *info) {}

  int getNroInfos() {
    return 0;
  }

  Metadata *getMetadata() {
    return md;
  }

  Timing *getMoveTiming(int moveIndex) {
    return routines[POSIT(moveIndex % NRO_ROUTINES)]->getTiming();
  }

  const char *getMove(int moveIndex) {
    return routines[POSIT(moveIndex % NRO_ROUTINES)]->getMove();
  }

  void performMove(int moveIndex) {
    performMove(getMove(moveIndex));
  }

  void performMove(const char *move) {
    const char *p = move;
    while ((p = performPose(p)) != NULL) {
    }
  }
};

#endif // BODY_INC
