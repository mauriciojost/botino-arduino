#ifndef LED_INC
#define LED_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>

enum LedConfigState {
  LedConfigOnState = 0,            // if the led is on
  LedConfigStateDelimiter // delimiter of the configuration states
};

class Led : public Actor {

private:
  bool currentValue;
  FreqConf freqConf;

public:
  Led();

  const char *getName();

  void cycle(bool cronMatches);
  void subCycle(float subCycle);
  void getActuatorValue(Value* value);

  int getNroProps();
  const char* getPropName(int propIndex);
  void setProp(int propIndex, SetMode set, const Value* targetValue, Value* actualValue);

  void getInfo(int infoIndex, Buffer<MAX_VALUE_STR_LENGTH>* info);
  int getNroInfos();

  FreqConf *getFrequencyConfiguration();
};

#endif // LED_INC
