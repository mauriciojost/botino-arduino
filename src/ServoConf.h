#ifndef SERVO_CONF_INC
#define SERVO_CONF_INC

#include <main4ino/Buffer.h>
#include <log4ino/Log.h>

#define CLASS_SERVOCONF "SC"

#define PROPS_SEPARATOR "/"
#define MAX_SERVO_STEPS 10

#define SERVO_INVERT_POS(p, i, r) (((i) ? ((r) - (p)) : (p)))
#define SERVO_CONF_SERIALIZED_MAX_LENGTH 16 // 123/567/8

class ServoConf {
private:
  int baseDeg;
  int rangeDeg;
  bool invert;
  float stepDeg;

  void logValues() {
    log(CLASS_SERVOCONF, Debug, "Base %d, range %d, invert %d, stepDeg %f", baseDeg, rangeDeg, (int)invert, stepDeg);
  }

  void updateStepDegrees() {
    stepDeg = (float)getRangeDegrees() / (MAX_SERVO_STEPS - 1);
  }

public:
  ServoConf() {
    baseDeg = 10;
    rangeDeg = 140;
    invert = false;
    stepDeg = (float)getRangeDegrees() / (MAX_SERVO_STEPS - 1);
    logValues();
  }
  ServoConf(const char *serialized) {
    Buffer b(16);
    b.load(serialized);
    baseDeg = atoi(strtok(b.getUnsafeBuffer(), PROPS_SEPARATOR));
    rangeDeg = atoi(strtok(NULL, PROPS_SEPARATOR));
    invert = (bool)atoi(strtok(NULL, PROPS_SEPARATOR));
    updateStepDegrees();
    logValues();
  }
  int getBaseDegrees() {
    return baseDeg;
  }
  int getRangeDegrees() {
    return rangeDeg;
  }
  bool getInvert() {
    return invert;
  }
  float getStepDegrees() {
    return stepDeg;
  }
  void setBase(int b) {
    baseDeg = b;
    logValues();
  }
  void setRange(int r) {
    rangeDeg = r;
    updateStepDegrees();
    logValues();
  }
  void setInvert(bool i) {
    invert = i;
    logValues();
  }
  void serialize(Buffer *dst) {
    dst->fill("%d" PROPS_SEPARATOR "%d" PROPS_SEPARATOR "%d" PROPS_SEPARATOR, baseDeg, rangeDeg, (int)invert);
  }

  /**
   * Given a position (between 0 and MAX_SERVO_STEPS -1) return the degrees.
   */
  int getTargetDegreesFromPosition(int po) {
    int deg = baseDeg + SERVO_INVERT_POS(((POSIT(po) % MAX_SERVO_STEPS) * stepDeg), invert, rangeDeg);
    log(CLASS_SERVOCONF, Debug, "(%d/%d/%d %f) Pos %d === %d deg", baseDeg, rangeDeg, (int)invert, stepDeg, po, deg);
    return deg;
  }
};

#endif // SERVO_CONF_INC
