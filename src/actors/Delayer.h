#ifndef DELAYER_INC
#define DELAYER_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Misc.h>

/**
* This actor works as a wrapper for other actors, delaying the cycle(match)
* call until a more convenient time.
* The typical use case is to reduce the electrical load in case 3 pumps are set
* to work at the same moment. Using this actor we can easily wrap the 3 pumps
* and specify an offset for each, let's say 0, 60 and 120. This way the first one
* will start as soon as a match comes, the second one 60 seconds after a match
* comes, and the third one 120 seconds after a match comes, balancing the electrical
* load.
* Note: this class must be serialized with care (contains pointers).
*/
class Delayer : public Actor {

private:
  Actor *actor; // wrapped actor on which to perform the match delay

  int offset; // offset in number of cycles to be applied once a real match arrives (useful to avoid electrical load peaks when activating
              // effectors like servos or pumps)

  bool matched;       // flag telling if there was a match in the past for which there is a count down
  int passTheMatchIn; // with [[matched]], tells how long until the match will be notified to the downstream actor

public:
  Delayer(int cowo);

  void setActor(Actor* a);

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

#endif // DELAYER_INC
