#ifndef IFTTT_INC
#define IFTTT_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Clock.h>
#include <main4ino/HttpCodes.h>
#include <main4ino/HttpMethods.h>
#include <main4ino/HttpResponse.h>
#include <main4ino/Misc.h>
#include <main4ino/ParamStream.h>
#include <main4ino/Table.h>

#define CLASS_IFTTT "IF"

#define IFTTT_API_URL_POS "http://maker.ifttt.com/trigger/%s/with/key/%s"
#define EVENT_NAME_MAX_LENGTH 32

#define CREDENTIAL_BUFFER_SIZE 64

#ifndef IFTTT_KEY
#define IFTTT_KEY "???"
#endif // IFTTT_KEY

enum IftttProps {
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
  HttpResponse (*httpMethod)(HttpMethod m, const char *url, Stream *body, Table *headers, const char *fingerprint);

  Buffer *iftttKey;
  Buffer *urlAuxBuffer;
  Table *headers;

public:
  Ifttt(const char *n) {
    name = n;

    iftttKey = new Buffer(CREDENTIAL_BUFFER_SIZE);
    iftttKey->load(IFTTT_KEY);
    urlAuxBuffer = new Buffer(128);
    initWifiFunc = NULL;
    httpMethod = NULL;
    md = new Metadata(n);
    headers = new Table(0);
  }

  const char *getName() {
    return name;
  }

  void act() {}

  void setKey(const char *k) {
    iftttKey->fill("%s", k);
  }

  const char *getKey() {
    return iftttKey->getBuffer();
  }

  void setInitWifi(bool (*f)()) {
    initWifiFunc = f;
  }

  void setHttpMethod(HttpResponse (*h)(HttpMethod m, const char *url, Stream *body, Table *headers, const char *fingerprint)) {
    httpMethod = h;
  }

  bool triggerEvent(const char *eventName) {
    if (initWifiFunc == NULL || httpMethod == NULL) {
      log(CLASS_IFTTT, Error, "Init needed");
      return false;
    }
    bool connected = initWifiFunc();
    if (connected) {
      urlAuxBuffer->fill(IFTTT_API_URL_POS, eventName, iftttKey->getBuffer());
      ParamStream s("{}");
      HttpResponse resp = httpMethod(HttpPost, urlAuxBuffer->getBuffer(), &s, headers, NULL);
      if (resp.code == HTTP_OK) {
        return true;
      }
    }
    return false;
  }

  void getSetPropValue(int propIndex, GetSetMode setMode, const Value *targetValue, Value *actualValue) {
    if (propIndex == IftttKeyProp) {
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
      case (IftttKeyProp):
        return SENSITIVE_PROP_PREFIX "iftttkey"; // with obfuscation (starts with _)
      default:
        return "";
    }
  }

  Metadata *getMetadata() {
    return md;
  }
};

#endif // IFTTT_INC
