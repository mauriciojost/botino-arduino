#ifndef MESSENGER_INC
#define MESSENGER_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Misc.h>
#include <ESP8266WiFi.h>

/**
* This actor connects to a server via WIFI.
*/
class Messenger : public Actor {

private:
  FreqConf freqConf;  // configuration of the frequency at which this actor will get triggered

public:
  Messenger();

  const char *getName();

  void cycle(bool cronMatches);
  void subCycle(float subCycle);
  int getActuatorValue();

  int getNroConfigs();
  void setConfig(int configIndex, char *retroMsg, bool set);

  void getInfo(int infoIndex, char *retroMsg);
  int getNroInfos();

  FreqConf *getFrequencyConfiguration();
};

#endif // MESSENGER_INC
