#ifndef IFTTT_INC
#define IFTTT_INC

#include <HttpCodes.h>
#include <actors/ParamStream.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Clock.h>
#include <main4ino/Misc.h>

#define CLASS_IFTTT "IF"

#define IFTTT_API_URL_POS "http://maker.ifttt.com/trigger/%s_%d/with/key/%s"

/**
 * This actor interacts wit IFTTT service.
 */
class Ifttt : public Actor {

private:
  const char *name;
  Timing timing;
  bool (*initWifiFunc)();
  int (*httpPost)(const char *url, const char *body, ParamStream *response);

  Buffer<64> iftttKey;
  Buffer<128> urlAuxBuffer;

public:
  Ifttt(const char *n) {
    name = n;
    initWifiFunc = NULL;
    httpPost = NULL;
    timing.setFrequency(Never);
  }

  const char *getName() {
    return name;
  }

  void act() {}

  void setKey(const char* k) {
  	iftttKey.fill("%s", k);
  }

  void setInitWifi(bool (*f)()) {
    initWifiFunc = f;
  }

  void setHttpPost(int (*h)(const char *url, const char *body, ParamStream *response)) {
    httpPost = h;
  }

  bool event(int eventNumber) {
  	if (initWifiFunc == NULL || httpPost == NULL) {
      log(CLASS_IFTTT, Error, "Init needed");
      return false;
  	}
    bool connected = initWifiFunc();
    if (connected) {
      urlAuxBuffer.fill(IFTTT_API_URL_POS, DEVICE_NAME, eventNumber, iftttKey.getBuffer());
      int errorCodePost = httpPost(urlAuxBuffer.getBuffer(), "{}", NULL);
      if (errorCodePost == HTTP_OK) {
        return true;
      }
    }
    return false;
  }


  void getSetPropValue(int propIndex, GetSetMode set, const Value *targetValue, Value *actualValue) {}

  int getNroProps() {
    return 0;
  }

  const char *getPropName(int propIndex) {
    return "";
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &timing;
  }
};

#endif // IFTTT_INC
