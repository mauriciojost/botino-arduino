#ifndef SERVO_CONF_INC
#define SERVO_CONF_INC

#include <main4ino/Buffer.h>
#include <log4ino/Log.h>

#define CLASS_SERVOCONF "SC"

#define PROPS_SEPARATOR "/"
#define MAX_SERVO_STEPS 10

#define SERVO_INVERT_POS(p, i, r) (((i) ? ((r) - (p)) : (p)))

class ServoConf {
private:
	int base;
	int range;
	bool invert;
	float stepDegrees;

	void logValues() {
    log(CLASS_SERVOCONF, Debug, "Base %d, range %d, invert %d, stepDeg %f", base, range, (int)invert, stepDegrees);
	}

public:
  ServoConf() {
    base = 10;
    range = 140;
    invert = false;
    stepDegrees = (float)getRangeDegrees() / (MAX_SERVO_STEPS - 1);
    logValues();
  }
  ServoConf(const char *serialized) {
  	Buffer b(16);
  	b.load(serialized);
    base = atoi(strtok(b.getUnsafeBuffer(), PROPS_SEPARATOR));
    range = atoi(strtok(NULL, PROPS_SEPARATOR));
    invert = (bool)atoi(strtok(NULL, PROPS_SEPARATOR));
    stepDegrees = (float)getRangeDegrees() / (MAX_SERVO_STEPS - 1);
    logValues();
  }
  int getBaseDegrees() { return base; }
  int getRangeDegrees() { return range; }
  bool getInvert() { return invert; }
  float getStepDegrees() { return stepDegrees; }
  void setBase(int b) { base = b; logValues(); }
  void setRange(int r) { range = r; logValues(); }
  void setInvert(bool i) { invert = i; logValues(); }
  void serialize(Buffer* dst) {
  	dst->fill("%d" PROPS_SEPARATOR "%d" PROPS_SEPARATOR "%d" PROPS_SEPARATOR, base, range, (int)invert);
  }

  /**
   * Given a position (between 0 and MAX_SERVO_STEPS -1) return the degrees.
   */
  int getTargetDegreesFromPosition(int po) {
  	int deg = getBaseDegrees()
  			+ SERVO_INVERT_POS(((POSIT(po) % MAX_SERVO_STEPS) * getStepDegrees()), getInvert(), getRangeDegrees());
    log(CLASS_SERVOCONF, Debug, "Pos %d === %d deg", po, deg);
    return deg;
  }

};

#endif // SERVO_CONF_INC

