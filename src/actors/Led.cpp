#include <actors/Led.h>

#define CLASS "Led"

Led::Led() {
  currentValue = false;
  freqConf.setFrequency(OnceEvery5Minutes);
}

const char *Led::getName() { return CLASS; }

void Led::cycle(bool cronMatches) { }

void Led::subCycle(float subCycle) { }

int Led::getActuatorValue() { return currentValue; }

void Led::setConfig(int configIndex, char *retroMsg, SetMode set, int* value) {
  switch (configIndex) {
    case (LedConfigOnState):
      if (set == SetNext) {
        currentValue = !currentValue;
      }
      if (set == SetValue) {
        currentValue = *value;
      }
      log(CLASS, Info, "SET LED: ", currentValue);
      *value = currentValue;
      break;
    default:
      break;
  }
}

int Led::getNroConfigs() { return LedConfigStateDelimiter; }

void Led::getInfo(int infoIndex, char *retroMsg) { }

int Led::getNroInfos() { return 0; }

FreqConf *Led::getFrequencyConfiguration() { return &freqConf; }
