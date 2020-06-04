#ifndef BODY_INC
#define BODY_INC

/**
 * Body
 *
 * Representative of the Botino physical robot.
 *
 * The robot can perform a few configurable routines with a given timing.
 *
 * The most important concept is the routine, which is made of:
 * - a timing condition
 * - a move to perform once the timing condition matches (a sequence of poses)
 *
 * A move can contain many poses. A pose can be: both arms up, both arms down, a given face (sad, smily) or even some
 * special conditions like illumination (red light) or control over the fan.
 *
 * Routines are configurable in both senses: timing and moves.
 *
 */

#include <actors/Ifttt.h>
#include <actors/Images.h>
#include <actors/Notifier.h>
#include <actors/Quotes.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Misc.h>
#include <main4ino/Timing.h>
#include <utils/Hexer.h>
#include <utils/Io.h>
#include <utils/Predictions.h>
#include <utils/Routine.h>

#define CLASS_BODY "BO"

#define NRO_ROUTINES 8

#define ARM_SLOW_FACTOR 4
#define ARM_NORMAL_FACTOR 2
#define ARM_FAST_FACTOR 1

enum PoseExecStatus { Unknown = 0, Interrupted, Failed, Invalid, Success, End };

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

#define MOVE_DANCE0 "Lwy.B09.B90.Lwn.B09.B90.Lwy.B55."
#define MOVE_DANCE1 "Lfy.Lyy.Lwy.A50.A05.Lry.Lwn.A00.A99.Lrn.Lwy.A90.A09.Lwn.Lyy.A90.A09."
#define MOVE_DANCE2 "A87.A78.L?.A87.A78.L?.A12.A21.L?.A12.A21.L?."
#define MOVE_DANCE3 "D/.D\\.Du.Dn."
#define MOVE_DANCE4 "A66.A88.A66.A88.A66.A88.A66.A88.A11.A33.A11.A33.A11.A33.A55.A33.A55.A33.A55.A33.C55.C31."
#define MOVE_DANCE5 "D5?"
#define MOVE_DANCE6 "D6?"
#define MOVE_DANCE7 "D7?"

#define MOVE_DANCE_U "A87.A78.L?.A87.A78.L?.A87.A78.L?.A87.A78.L?."
#define MOVE_DANCE_n "A12.A21.L?.A12.A21.L?.A12.A21.L?.A12.A21.L?."
#define MOVE_DANCE_BACK_SLASH "A71.A82.L?.A71.A82.L?.A71.A82.L?.A71.A82."
#define MOVE_DANCE_FORW_SLASH "A17.A28.L?.A17.A28.L?.A17.A28.L?.A17.A28."

#define TIMER_MSG_X 7
#define TIMER_MSG_Y 0

// Create images with:
// https://docs.google.com/spreadsheets/d/1jXa9mFxeiN_bUji_WiCPKO_gB6pxQUeQ5QxgoSINqdc/edit#gid=0
uint8_t IMG_CRAZY[] = {0x00, 0x00, 0x7F, 0x00, 0x41, 0x3E, 0x41, 0x22, 0x49, 0x2A, 0x41, 0x22, 0x7F, 0x3E, 0x00, 0x00};
uint8_t IMG_SMILY[] = {0x00, 0x00, 0x02, 0x40, 0x12, 0x48, 0x12, 0x48, 0x08, 0x10, 0x08, 0x10, 0x04, 0x20, 0x03, 0xC0};
uint8_t IMG_NORMAL[] = {0x00, 0x00, 0x02, 0x40, 0x02, 0x40, 0x00, 0x00, 0x01, 0x00, 0x01, 0x80, 0x04, 0x20, 0x03, 0xC0};
uint8_t IMG_ANGRY[] = {0x00, 0x00, 0x70, 0x0E, 0x49, 0x12, 0x55, 0x2A, 0x45, 0x22, 0x7D, 0xBE, 0x00, 0x00, 0x03, 0xC0};
uint8_t IMG_SAD[] = {0x08, 0x10, 0x08, 0x10, 0x09, 0x94, 0x20, 0x00, 0x03, 0xC0, 0x0C, 0x30, 0x10, 0x08, 0x20, 0x04};
uint8_t IMG_SLEEPY[] = {0x00, 0x00, 0x00, 0x00, 0x12, 0x48, 0x0C, 0x30, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00};

class Body : public Actor {

private:
  const char *name;
  Metadata *md;
  void (*arms)(int left, int right, int perFactor);
  bool (*sleepInterruptable)(time_t cycleBegin, time_t periodSec);
  bool (*button)();
  Notifier *notifier;
  void (*iosFunc)(char led, IoMode v);
  Quotes *quotes;
  Images *images;
  Ifttt *ifttt;
  Routine *routines[NRO_ROUTINES];

  PoseExecStatus dance(char c0) {
    log(CLASS_BODY, Debug, "Dance '%c'", c0);
    PoseExecStatus s = Unknown;
    switch (c0) {
      case '0':
        s = performMove(MOVE_DANCE0);
        break;
      case '1':
        s = performMove(MOVE_DANCE1);
        break;
      case '2':
        s = performMove(MOVE_DANCE2);
        break;
      case '3':
        s = performMove(MOVE_DANCE3);
        break;
      case '4':
        s = performMove(MOVE_DANCE4);
        break;
      case '5':
        s = performMove(MOVE_DANCE5);
        break;
      case '6':
        s = performMove(MOVE_DANCE6);
        break;
      case '7':
        s = performMove(MOVE_DANCE7);
        break;
      case 'n':
        s = performMove(MOVE_DANCE_n);
        break;
      case 'u':
        s = performMove(MOVE_DANCE_U);
        break;
      case '\\':
        s = performMove(MOVE_DANCE_BACK_SLASH);
        break;
      case '/':
        s = performMove(MOVE_DANCE_FORW_SLASH);
        break;
      default:
        s = Invalid;
        break;
    }
    // this pose belongs to a move
    // this pose executes a sub move
    // the end of the sub move (variable s) does not mean that this
    // pose is the end of the outer move, that's why the line below
    return (s == End ? Success : s);
  }

  PoseExecStatus face(char c0) {
    switch (c0) {
      case '0':
        notifier->lcdImg('c', images->get(0)); // custom 0
        return Success;
      case '1':
        notifier->lcdImg('c', images->get(1)); // custom 1
        return Success;
      case '2':
        notifier->lcdImg('c', images->get(2)); // custom 2
        return Success;
      case '3':
        notifier->lcdImg('c', images->get(3)); // custom 3
        return Success;
      case '_':
        notifier->lcdImg('_', NULL); // dim
        return Success;
      case '-':
        notifier->lcdImg('-', NULL); // bright
        return Success;
      case 'w':
        notifier->lcdImg('w', NULL); // white
        return Success;
      case 'b':
        notifier->lcdImg('b', NULL); // black
        return Success;
      case 'l':
        notifier->lcdImg('l', NULL); // clear
        return Success;
      case 'r':
        notifier->lcdImg('c', IMG_CRAZY); // crazy
        return Success;
      case 's':
        notifier->lcdImg('c', IMG_SMILY); // smile
        return Success;
      case 'S':
        notifier->lcdImg('c', IMG_SAD); // sad
        return Success;
      case 'n':
        notifier->lcdImg('c', IMG_NORMAL); // normal
        return Success;
      case 'a':
        notifier->lcdImg('c', IMG_ANGRY); // angry
        return Success;
      case 'z':
        notifier->lcdImg('c', IMG_SLEEPY); // sleepy
        return Success;
      default:
        return Invalid;
    }
  }

  bool isInitialized() {
    bool init = arms != NULL && button != NULL && sleepInterruptable != NULL && iosFunc != NULL && notifier != NULL && quotes != NULL &&
                images != NULL && ifttt != NULL;
    return init;
  }

  int getInt(char c) {
    // '0' -> 0, ..., '9' -> 9
    return ABSOL(c - '0');
  }

  int getIosState(char c) {
    if (c == 'n' || c == 'N' || c == '0') {
      return IoOff;
    } else if (c == 'y' || c == 'Y' || c == '1') {
      return IoOn;
    } else if (c == 't' || c == 'T') {
      return IoToggle;
    } else {
      return IoOff;
    }
  }

  int nextPoseStrLen(const char *p) {
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

  PoseExecStatus performPoseLen(const char *pose, int len) {

    char c0 = 'x', c1 = 'x';
    int i0 = 0;

    log(CLASS_BODY, Debug, "Performing pose: '%.*s'", len, pose);
    if (sleepInterruptable(now(), 0)) { // check if this pose's execution has to be interrupted
      return Interrupted;
    }

    if (sscanf(pose, "A%c%c.", &c0, &c1) == 2) {
      // ARMS FAST
      int l = getInt(c0);
      int r = getInt(c1);
      log(CLASS_BODY, Debug, "Armsf %d&%d", l, r);
      arms(l, r, ARM_FAST_FACTOR);
      return Success;
    } else if (sscanf(pose, "B%c%c.", &c0, &c1) == 2) {
      // ARMS MEDIUM
      int l = getInt(c0);
      int r = getInt(c1);
      log(CLASS_BODY, Debug, "Armsn %d&%d", l, r);
      arms(l, r, ARM_NORMAL_FACTOR);
      return Success;
    } else if (sscanf(pose, "C%c%c.", &c0, &c1) == 2) {
      // ARMS SLOW
      int l = getInt(c0);
      int r = getInt(c1);
      log(CLASS_BODY, Debug, "Armss %d&%d", l, r);
      arms(l, r, ARM_SLOW_FACTOR);
      return Success;
    } else if (sscanf(pose, "D%c.", &c0) == 1) {
      // DANCE
      return dance(c0);
    } else if (sscanf(pose, "F%c.", &c0) == 1) {
      // FACES
      return face(c0);
    } else if (sscanf(pose, "I%c", &c0) == 1) {
      // IFTTT (by name)
      Buffer evt(MOVE_STR_LENGTH, pose + 1);
      evt.replace('.', 0);
      log(CLASS_BODY, Debug, "Event '%s'", evt.getBuffer());
      bool suc = ifttt->triggerEvent(evt.getBuffer());
      if (!suc) {
        notifier->message(0, 1, "Failed ifttt\n'%s'", evt.getBuffer());
      }
      return Success;
    } else if (sscanf(pose, "L%c%c.", &c0, &c1) == 2) {
      // IO (LEDS / FAN)
      int b = getIosState(c1);
      iosFunc(c0, (IoMode)b);
      return Success;
    } else if (sscanf(pose, "L%c.", &c0) == 1) {
      // IO (LEDS / FAN)
      switch (c0) {
        case '?': {
          iosFunc('r', (IoMode)random(2));
          iosFunc('w', (IoMode)random(2));
          iosFunc('y', (IoMode)random(2));
          return Success;
        }
        default:
          return Invalid;
      }
    } else if (sscanf(pose, "M%c%c.", &c0, &c1) == 2) {
      // MESSAGES
      switch (c0) {
        case 'c': {
          log(CLASS_BODY, Debug, "Msg clock");
          int h = GET_HOURS(getTiming()->getCurrentTime());
          int m = GET_MINUTES(getTiming()->getCurrentTime());
          Buffer t(6, "");
          t.fill("%02d:%02d", h, m);
          notifier->message(0, getInt(c1), t.getBuffer());
          return Success;
        }
        case 'k': {
          log(CLASS_BODY, Debug, "Msg date");
          long t = getTiming()->getCurrentTime();
          Buffer b(18, "");
          b.fill("%4d-%02d-%02d\n%02d:%02d", GET_YEARS(t), GET_MONTHS(t), GET_DAYS(t), GET_HOURS(t), GET_MINUTES(t));
          notifier->message(0, getInt(c1), b.getBuffer());
          return Success;
        }
        case 'q': {
          log(CLASS_BODY, Debug, "Msg quote");
          int i = random(NRO_QUOTES);
          notifier->message(0, getInt(c1), quotes->getQuote(i));
          return Success;
        }
        case 'p': {
          log(CLASS_BODY, Debug, "Msg prediction");
          Buffer pr(200, "");
          Predictions::getPrediction(&pr);
          notifier->message(0, getInt(c1), pr.getBuffer());
          return Success;
        }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6': {
          int size = getInt(c0);
          Buffer msg(MOVE_STR_LENGTH, pose + 2);
          msg.replace('.', 0);
          log(CLASS_BODY, Debug, "Msg '%s'", msg.getBuffer());
          notifier->message(0, size, msg.getBuffer());
          return Success;
        }
        default:
          return Invalid;
      }
    } else if (sscanf(pose, "N%c", &c0) == 1) {
      // NOTIFICATION
      Buffer msg(MOVE_STR_LENGTH, pose + 1);
      msg.replace('.', 0);
      log(CLASS_BODY, Debug, "Not '%s'", msg.getBuffer());
      notifier->notification(msg.getBuffer());
      return Success;
    } else if (sscanf(pose, "W%c.", &c0) == 1) {
      // WAIT
      int v = getInt(c0);
      log(CLASS_BODY, Debug, "Wait %d s", v);
      if (sleepInterruptable(now(), v)) {
        return Interrupted;
      } else {
        return Success;
      }
    } else if (sscanf(pose, "T%d.", &i0) == 1) {
// TIMER
#define RESP 10
      log(CLASS_BODY, Debug, "Timer %d s", i0);
      for (int i = 0; i < i0 * RESP; i++) {
        if (button())
          break;
        if (i % RESP == 0) {
          notifier->message(TIMER_MSG_X, TIMER_MSG_Y, "%d", i / RESP);
        }
        delay(1000 / RESP);
      }
      notifier->message(TIMER_MSG_X, TIMER_MSG_Y, "OK!");
      delay(500);
      return Success;
    } else if (sscanf(pose, "Z%c", &c0) == 1) {
      // POWER OFF
      return z();
    } else {
      return Invalid;
    }
  }

public:
  /**
   * Perform a given pose
   *
   * Returns the next pose in the string or NULL if no more poses to perform.
   */
  const char *performPose(const char *pose, PoseExecStatus *status) {

    int poseLen = nextPoseStrLen(pose);
    (*status) = Unknown;

    if (poseLen <= 0) { // invalid number of chars for next pose
      (*status) = End;
    } else if (poseLen > 0) {
      (*status) = performPoseLen(pose, poseLen);
    }

    switch (*status) {
      case Success:
        log(CLASS_BODY, Debug, "Done pose (first '%.*s')", poseLen, pose);
        return pose + poseLen + 1;
      case Interrupted:
        log(CLASS_BODY, Warn, "Interrupted pose '%s'", pose);
        return NULL;
      case Invalid:
        notifier->message(0, 1, "Invalid pose '%s'", pose);
        return NULL;
      case Failed:
        notifier->message(0, 1, "Failed pose: '%s'", pose);
        return NULL;
      case End:
        log(CLASS_BODY, Debug, "No more poses");
        return NULL;
      default:
        log(CLASS_BODY, Error, "Unexpected pose status '%s'", pose);
        return NULL;
    }
  }

  Body(const char *n) {
    name = n;
    arms = NULL;
    sleepInterruptable = NULL;
    button = NULL;
    iosFunc = NULL;
    quotes = NULL;
    images = NULL;
    ifttt = NULL;
    notifier = NULL;
    md = new Metadata(n);
    md->getTiming()->setFreq("~1m");
    for (int i = 0; i < NRO_ROUTINES; i++) {
      routines[i] = new Routine(i);
      routines[i]->set("never", "Z.");
    }

    // Overwrite last to setup clock
    routines[NRO_ROUTINES - 1]->set("~2m", "Mc3."); // show time
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

  void
  setup(void (*a)(int left, int right, int f), void (*f)(char led, IoMode v), bool (*i)(time_t cycleBegin, time_t periodSec), bool (*b)()) {
    arms = a;
    iosFunc = f;
    sleepInterruptable = i;
    button = b;
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
        routines[i]->set(targetValue);
      }
      if (actualValue != NULL) {
        actualValue->deserializeFromValue(routines[i]->timingMove);
      }
    }
    if (m != GetValue) {
      getMetadata()->changed();
    }
  }

  int getNroProps() {
    return BodyPropsDelimiter;
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

  PoseExecStatus performMove(int moveIndex) {
    return performMove(getMove(moveIndex));
  }

  PoseExecStatus performMove(const char *move) {
    log(CLASS_BODY, Debug, "Move '%s' started", move);
    int i = 0;
    const char *p = move;
    PoseExecStatus status = Unknown;
    log(CLASS_BODY, Debug, "Performing move '%s'", move);
    while ((p = performPose(p, &status)) != NULL) {
      i++;
      log(CLASS_BODY, Debug, "- pose %d: (first of)'%s'...", i, p);
      if (status != Success && status != End) {
        log(CLASS_BODY, Warn, "Bad move: %d '%s'", (int)status, p);
        break;
      }
    }
    log(CLASS_BODY, Debug, "Move '%s' finished: %d poses executed (s=%d)", move, i, status);
    return status;
  }

  /**
   * Sleep method (low power consumption).
   *
   * Initialization safe.
   */
  PoseExecStatus z() {
    log(CLASS_BODY, Debug, "ZzZ...");
    if (!isInitialized()) {
      log(CLASS_BODY, Warn, "No init!");
      return Failed;
    }
    notifier->clearLcd();
    iosFunc('*', IoOff);
    arms(0, 0, ARM_NORMAL_FACTOR);
    return Success;
  }
};

#endif // BODY_INC
