#include <actors/Led.h>

#define CLASS "LE"

Led::Led(const char* n): freqConf(OnceEvery5Minutes) {
  name = n;
  currentValue = false;
}

const char *Led::getName() {
  return name;
}

void Led::cycle(bool cronMatches) { }

void Led::subCycle(float subCycle) { }

void Led::getActuatorValue(Value* value) {
  if (value != NULL) {
    Integer i(currentValue?1:0);
    value->load(&i);
  }
}

const char* Led::getPropName(int propIndex) {
  switch (propIndex) {
    case (LedConfigOnState): return "on";
    default: return "";
  }
}

void Led::setProp(int propIndex, SetMode setMode, const Value* targetValue, Value* actualValue) {
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

int Led::getNroProps() { return LedConfigStateDelimiter; }

void Led::getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH>* info) {
  Boolean i(currentValue);
  info->load(&i);
}

int Led::getNroInfos() { return 1; }

FreqConf *Led::getFrequencyConfiguration() { return &freqConf; }
