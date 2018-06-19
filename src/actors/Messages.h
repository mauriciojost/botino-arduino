#ifndef ACT_MESSAGES_INC
#define ACT_MESSAGES_INC

#include <main4ino/Misc.h>
#include <Hexer.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Value.h>
#include <actors/Quotes.h>
#include <actors/Images.h>

#define CLASS_MESSAGES "BO"
#define MSG_MAX_LENGTH 32

#define NRO_MSGS 4

enum MessagesConfigState {
  MessagesConfigMsg0 = 0,      // message 0, examples are "HI", "HELLO"
  MessagesConfigMsg1,          // message 1
  MessagesConfigMsg2,          // message 2
  MessagesConfigMsg3,          // message 3
  MessagesConfigStateDelimiter // delimiter of the configuration states
};

class Messages : public Actor {

private:
  const char *name;
  Timing timing;
  Buffer<MSG_MAX_LENGTH> *msgs[NRO_MSGS];

public:
  Messages(const char *n) : timing(Never) {
    name = n;
    for (int i = 0; i < NRO_MSGS; i++) {
      msgs[i] = new Buffer<MSG_MAX_LENGTH>("");
    }
  }

  const char *getName() {
    return name;
  }

  void act() {

  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (MessagesConfigMsg0):
        return "msg0";
      case (MessagesConfigMsg1):
        return "msg1";
      case (MessagesConfigMsg2):
        return "msg2";
      case (MessagesConfigMsg3):
        return "msg3";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode setMode, const Value *targetValue, Value *actualValue) {
    if (propIndex >= MessagesConfigMsg0 && propIndex < (NRO_MSGS + MessagesConfigMsg0)) {
      int i = (int)propIndex - (int)MessagesConfigMsg0;
      setPropValue(setMode, targetValue, actualValue, msgs[i]);
    }
  }

  int getNroProps() {
    return MessagesConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &timing;
  }

  const char *get(int i) {
    return msgs[POSIT(i) % NRO_MSGS]->getBuffer();
  }

};

#endif // ACT_MESSAGES_INC
