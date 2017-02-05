#include <actors/Delayer.h>

#define CLASS "Delayer"

Delayer::Delayer(int o) {
  actor = NULL;
  offset = o;
  matched = false;
  passTheMatchIn = 0;
}

const char *Delayer::getName() {
  return "hey";
}

void Delayer::cycle(bool cronMatches) {
  
}

void Delayer::subCycle(float subCycle) {
  
}

int Delayer::getActuatorValue() {
  return 0;
}

void Delayer::setConfig(int configIndex, char *retroMsg, bool set) {
  
}

int Delayer::getNroConfigs() {
  return 0;
}

void Delayer::getInfo(int infoIndex, char *retroMsg) {
  
}

int Delayer::getNroInfos() {
  return 0;
}

FreqConf *Delayer::getFrequencyConfiguration() {
  return &freqConf;
}
