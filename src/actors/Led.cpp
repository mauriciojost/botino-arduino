#include <actors/Led.h>

#define CLASS "Led"

Led::Led(): freqConf(OnceEvery5Minutes) {
  currentValue = false;
}

const char *Led::getName() { return CLASS; }

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
    case (LedConfigOnState): return "configon";
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
        Integer i;
        i.load(targetValue);
        currentValue = i.get();
      }
      if (actualValue != NULL) {
        Integer i(currentValue);
        actualValue->load(&i);
      }
      log(CLASS, Info, "SET LED: ", currentValue);
      break;
    default:
      break;
  }
}

int Led::getNroProps() { return LedConfigStateDelimiter; }

void Led::getInfo(int infoIndex, Buffer<MAX_VALUE_STR_LENGTH>* info) { }

int Led::getNroInfos() { return 0; }

FreqConf *Led::getFrequencyConfiguration() { return &freqConf; }
