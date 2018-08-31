#ifndef ACT_MESSAGES_INC
#define ACT_MESSAGES_INC

/**
 * Messages
 *
 * Holds custom messages.
 *
 * Each message is at most MSG_MAX_LENGTH long, and this actor supports NRO_MSGS messages.
 *
 */
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Misc.h>

#include <main4ino/Value.h>

#define CLASS_MESSAGES "BO"
#define MSG_MAX_LENGTH 32

#define NRO_MSGS 4

enum MessagesProps {
  MessagesMsg0Prop = 0,  // string, message 0, examples are "HI", "HELLO"
  MessagesMsg1Prop,      // string, message 1
  MessagesMsg2Prop,      // string, message 2
  MessagesMsg3Prop,      // string, message 3
  MessagesPropsDelimiter // delimiter of the configuration states
};

class Messages : public Actor {

private:
  const char *name;
  Metadata* md;
  Buffer<MSG_MAX_LENGTH> *msgs[NRO_MSGS];

public:
  Messages(const char *n) {
    name = n;
    for (int i = 0; i < NRO_MSGS; i++) {
      msgs[i] = new Buffer<MSG_MAX_LENGTH>("");
    }
    md = new Metadata(n);
  }

  const char *getName() {
    return name;
  }

  void act() {}

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (MessagesMsg0Prop):
        return "msg0";
      case (MessagesMsg1Prop):
        return "msg1";
      case (MessagesMsg2Prop):
        return "msg2";
      case (MessagesMsg3Prop):
        return "msg3";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode setMode, const Value *targetValue, Value *actualValue) {
    if (propIndex >= MessagesMsg0Prop && propIndex < (NRO_MSGS + MessagesMsg0Prop)) {
      int i = (int)propIndex - (int)MessagesMsg0Prop;
      setPropValue(setMode, targetValue, actualValue, msgs[i]);
    }
  }

  int getNroProps() {
    return MessagesPropsDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Metadata *getMetadata() {
    return md;
  }

  const char *get(int i) {
    return msgs[POSIT(i) % NRO_MSGS]->getBuffer();
  }
};

#endif // ACT_MESSAGES_INC
