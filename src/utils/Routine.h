#ifndef ROUTINE_INC
#define ROUTINE_INC

#include <main4ino/Buffer.h>
#include <main4ino/Timing.h>

#define TIMING_SEPARATOR ':'

#define TIMING_AND_SEPARATOR_STR_LEN (TIMING_STR_LEN + 1)

#define MOVE_STR_LENGTH (32 + TIMING_AND_SEPARATOR_STR_LEN)

class Routine {
public:
  Buffer *timingMove;
  Timing *timing;
  char name[3];
  Routine(int idx) {
    timingMove = new Buffer(MOVE_STR_LENGTH);
    name[0] = 'R';
    name[1] = idx + '0';
    name[2] = 0;
    timing = new Timing(name);
  }
  void set(const Value *v) {
    Buffer aux(MOVE_STR_LENGTH);
    aux.deserializeFromValue(v);
    const char *mv = aux.since(TIMING_SEPARATOR);
    aux.replace(TIMING_SEPARATOR, 0);
    const char *tm = aux.getBuffer();
    set(tm, mv);
  }
  void set(const char *tmg, const char *mov) {
    if (tmg == NULL || strlen(tmg) == 0) {
      timingMove->fill("error%c%s", TIMING_SEPARATOR, mov);
    } else {
      timing->setFreq(tmg);
      timingMove->fill("%s%c%s", timing->getFreq(), TIMING_SEPARATOR, mov);
    }
  }
  const char *getMove() {
    const char *m = timingMove->since(TIMING_SEPARATOR);
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

#endif // ROUTINE_INC
