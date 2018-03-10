#ifndef MESSENGER_INC
#define MESSENGER_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/WebBot.h>
#include <main4ino/Misc.h>
#ifndef UNIT_TEST
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif // UNIT_TEST
#include <main4ino/Bot.h>

#define MAX_URL_EFF_LENGTH 100

/**
* This actor connects to a server via WIFI.
*/
class Messenger : public Actor {

private:
  const char* name;
  WebBot* bot;
  FreqConf freqConf;  // configuration of the frequency at which this actor will get triggered
  Buffer<MAX_JSON_STR_LENGTH> staticBuffer;
  Buffer<MAX_URL_EFF_LENGTH> staticUrl;

public:
  Messenger(const char* n);

  const char *getName();

  void cycle(bool cronMatches);
  void subCycle(float subCycle);
  void getActuatorValue(Value* value);

  int getNroProps();
  const char* getPropName(int propIndex);
  void setProp(int propIndex, SetMode set, const Value* targetValue, Value* actualValue);

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH>* info);
  int getNroInfos();

  FreqConf *getFrequencyConfiguration();

  void setBot(WebBot* b);
  void updateBotProperties();
  void updateClockProperties();
  void setUpDweetClient(HTTPClient* client, Buffer<MAX_URL_EFF_LENGTH> *url);

};

#endif // MESSENGER_INC
