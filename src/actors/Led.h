#ifndef LED_INC
#define LED_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS "LE"

enum LedConfigState {
  LedConfigOnState = 0,            // if the led is on
  LedConfigStateDelimiter // delimiter of the configuration states
};

class Led : public Actor {

private:
  const char* name;
  bool currentValue;
  Timing freqConf;
  int pin;
  void (*digitalWriteFunc)(unsigned char, unsigned char);

public:

  Led(const char* n, int p): freqConf(OnceEvery10Seconds) {
    name = n;
    currentValue = false;
    pin = p;
    digitalWriteFunc = NULL;
  }

  const char *getName() {
    return name;
  }

  void setDigitalWriteFunction(void (*d)(unsigned char, unsigned char)) {
  	digitalWriteFunc = d;
  }

  void cycle() {
    if (freqConf.matches()) {
  		if (digitalWriteFunc != NULL) {
  			digitalWriteFunc(pin, currentValue);
  		}
  	}
  }

  const char* getPropName(int propIndex) {
    switch (propIndex) {
      case (LedConfigOnState): return "on";
      default: return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value* targetValue, Value* actualValue) {
    switch (propIndex) {
      case (LedConfigOnState):
        if (setMode == SetNext) {
          currentValue = !currentValue;
        }
        if (setMode == SetValue) {
          Boolean b(targetValue);
          currentValue = b.get();
        }
        if (setMode != DoNotSet) {
          log(CLASS, Info, "Led: %s", name);
          log(CLASS, Info, " set: %d", currentValue);
        }
        if (actualValue != NULL) {
          Boolean b(currentValue);
          actualValue->load(&b);
        }
        break;
      default:
        break;
    }
  }

  int getNroProps() { return LedConfigStateDelimiter; }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH>* info) {
    Boolean i(currentValue);
    info->load(&i);
  }

  int getNroInfos() { return 1; }

  Timing *getFrequencyConfiguration() { return &freqConf; }

};

#endif // LED_INC
