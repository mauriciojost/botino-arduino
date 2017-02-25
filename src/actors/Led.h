#ifndef LED_INC
#define LED_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>

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
  int getActuatorValue();

  int getNroConfigs();
  void setConfig(int configIndex, char *retroMsg, SetMode set, int* value = 0);

  void getInfo(int infoIndex, char *retroMsg);
  int getNroInfos();

  FreqConf *getFrequencyConfiguration();
};

#endif // LED_INC
