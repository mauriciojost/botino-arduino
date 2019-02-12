#ifndef ROUTINE_INC
#define ROUTINE_INC

#define TIMING_SEPARATOR ':'

#define TIMING_AND_SEPARATOR_STR_LEN (TIMING_STR_LEN + 1)

#define MOVE_STR_LENGTH (32 + TIMING_AND_SEPARATOR_STR_LEN)

class Routine {
public:
  Buffer *timingMove;
  Timing *timing;
  Routine(const char *n) {
    timingMove = new Buffer(MOVE_STR_LENGTH);
    timing = new Timing(n);
  }
  void set(const Value *v) {
    Buffer aux(MOVE_STR_LENGTH);
    aux.load(v);
    const char* mv = aux.since(TIMING_SEPARATOR);
    aux.replace(TIMING_SEPARATOR, 0);
    const char* tm = aux.getBuffer();
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
