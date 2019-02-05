#ifndef SERVO_CONF_INC
#define SERVO_CONF_INC

#include <main4ino/Buffer.h>

#define PROPS_SEPARATOR "/"
#define MAX_SERVO_STEPS 10

#define SERVO_INVERT_POS(p, i, r) (((i) ? ((r) - (p)) : (p)))

class ServoConf {
private:
	int base;
	int range;
	bool invert;
	float stepDegrees;

public:
  ServoConf() {
    base = 10;
    range = 140;
    invert = false;
    stepDegrees = (float)getRangeDegrees() / (MAX_SERVO_STEPS - 1);
  }
  ServoConf(const char *s) {
  	Buffer b(16);
  	b.load(s);
    base = atoi(strtok(b.getUnsafeBuffer(), PROPS_SEPARATOR));
    range = atoi(strtok(NULL, PROPS_SEPARATOR));
    invert = (bool)atoi(strtok(NULL, PROPS_SEPARATOR));
    stepDegrees = (float)getRangeDegrees() / (MAX_SERVO_STEPS - 1);
  }
  int getBaseDegrees() { return base; }
  int getRangeDegrees() { return range; }
  bool getInvert() { return invert; }
  float getStepDegrees() { return stepDegrees; }

  /**
   * Given a position (between 0 and MAX_SERVO_STEPS -1) return the degrees.
   */
  int getTargetDegreesFromPosition(int po) {
  	return getBaseDegrees()
  			+ SERVO_INVERT_POS(((POSIT(po) % MAX_SERVO_STEPS) * getStepDegrees()), getInvert(), getRangeDegrees());
  }

};

#endif // SERVO_CONF_INC

