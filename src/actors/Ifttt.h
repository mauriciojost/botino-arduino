#ifndef IFTTT_INC
#define IFTTT_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Clock.h>
#include <main4ino/HttpCodes.h>
#include <main4ino/Misc.h>
#include <main4ino/ParamStream.h>
#include <main4ino/Table.h>

#define CLASS_IFTTT "IF"

#define IFTTT_API_URL_POS "http://maker.ifttt.com/trigger/%s/with/key/%s"
#define NRO_EVENTS 4
#define EVENT_NAME_MAX_LENGTH 32

#define CREDENTIAL_BUFFER_SIZE 64

#ifndef IFTTT_KEY
#define IFTTT_KEY "???"
#endif // IFTTT_KEY


enum IftttProps {
  IftttEvtName0Prop = 0,
  IftttEvtName1Prop,
  IftttEvtName2Prop,
  IftttEvtName3Prop,
  IftttKeyProp,       // ifttt key credential (for webhook, sensitive)
  IftttPropsDelimiter // delimiter of the configuration states
};

/**
 * This actor interacts wit IFTTT service.
 */
class Ifttt : public Actor {

private:
  const char *name;
  Metadata *md;
  bool (*initWifiFunc)();
  int (*httpPost)(const char *url, const char *body, ParamStream *response, Table *headers);

  Buffer *iftttKey;
  Buffer *urlAuxBuffer;
  Buffer *eventNames[NRO_EVENTS];
  Table *headers;

public:
  Ifttt(const char *n) {
    name = n;

    iftttKey = new Buffer(CREDENTIAL_BUFFER_SIZE);
    iftttKey->load(IFTTT_KEY);
    urlAuxBuffer = new Buffer(128);
    initWifiFunc = NULL;
    httpPost = NULL;
    md = new Metadata(n);
    for (int i = 0; i < NRO_EVENTS; i++) {
      eventNames[i] = new Buffer(EVENT_NAME_MAX_LENGTH);
      eventNames[i]->fill("event_%d", i);
    }
    headers = new Table(0, 0, 0);
  }

  const char *getName() {
    return name;
  }

  void act() {}

  void setKey(const char *k) {
    iftttKey->fill("%s", k);
  }

  const char* getKey() {
    return iftttKey->getBuffer();
  }

  void setInitWifi(bool (*f)()) {
    initWifiFunc = f;
  }

  void setHttpPost(int (*h)(const char *url, const char *body, ParamStream *response, Table *headers)) {
    httpPost = h;
  }

  const char *getEventName(int eventNumber) {
    int safeEvtNumber = POSIT(eventNumber % NRO_EVENTS);
    return eventNames[safeEvtNumber]->getBuffer();
  }

  bool triggerEvent(const char *eventName) {
    if (initWifiFunc == NULL || httpPost == NULL) {
      log(CLASS_IFTTT, Error, "Init needed");
      return false;
    }
    bool connected = initWifiFunc();
    if (connected) {
      urlAuxBuffer->fill(IFTTT_API_URL_POS, eventName, iftttKey->getBuffer());
      int errorCodePost = httpPost(urlAuxBuffer->getBuffer(), "{}", NULL, headers);
      if (errorCodePost == HTTP_OK) {
        return true;
      }
    }
    return false;
  }

  bool triggerEvent(int eventNumber) {
    int safeEvtNumber = POSIT(eventNumber % NRO_EVENTS);
    const char *eventName = getEventName(safeEvtNumber);
    return triggerEvent(eventName);
  }

  void getSetPropValue(int propIndex, GetSetMode setMode, const Value *targetValue, Value *actualValue) {
    if (propIndex >= IftttEvtName0Prop && propIndex < (NRO_EVENTS + IftttEvtName0Prop)) {
      int i = (int)propIndex - (int)IftttEvtName0Prop;
      setPropValue(setMode, targetValue, actualValue, eventNames[i]);
    } else if (propIndex == IftttKeyProp) {
      setPropValue(setMode, targetValue, actualValue, iftttKey);
    }
    if (setMode != GetValue) {
      getMetadata()->changed();
    }
  }

  int getNroProps() {
    return IftttPropsDelimiter;
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (IftttEvtName0Prop):
        return "evt0";
      case (IftttEvtName1Prop):
        return "evt1";
      case (IftttEvtName2Prop):
        return "evt2";
      case (IftttEvtName3Prop):
        return "evt3";
      case (IftttKeyProp):
        return "_iftttkey"; // with obfuscation (starts with _)
      default:
        return "";
    }
  }

  void getInfo(int infoIndex, Buffer *info) {}

  int getNroInfos() {
    return 0;
  }

  Metadata *getMetadata() {
    return md;
  }
};

#endif // IFTTT_INC
