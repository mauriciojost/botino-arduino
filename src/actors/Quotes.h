#ifndef QUOTES_INC
#define QUOTES_INC

#include <HttpCodes.h>
#include <actors/ParamStream.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Misc.h>
#include <main4ino/Value.h>

#define CLASS_QUOTES "QU"

#define QUOTE_MAX_LENGTH 128
#define NRO_QUOTES 6

#define URL_QUOTES "http://api.forismatic.com/api/1.0/POST?method=getQuote&format=text&lang=en"

/**
 * Retrieve quotes from the internet.
 */
class Quotes : public Actor {

private:
  const char *name;
  Timing timing;
  Buffer<QUOTE_MAX_LENGTH> *quotes[NRO_QUOTES];
  int (*httpGet)(const char *url, ParamStream *response);
  bool (*initWifiFunc)();

  bool isInitialized() {
    return (httpGet != NULL && initWifiFunc != NULL);
  }

public:
  Quotes(const char *n) {
    name = n;
    httpGet = NULL;
    initWifiFunc = NULL;
    timing.setFrequency(TwicePerDay);
    for (int i = 0; i < NRO_QUOTES; i++) {
      quotes[i] = new Buffer<QUOTE_MAX_LENGTH>("Damn! No quote yet! :(");
    }
  }

  const char *getName() {
    return name;
  }

  void setHttpGet(int (*h)(const char *url, ParamStream *response)) {
    httpGet = h;
  }

  void setInitWifi(bool (*f)()) {
    initWifiFunc = f;
  }

  void act() {
    if (!isInitialized()) {
      log(CLASS_QUOTES, Warn, "No init!");
      return;
    }
    if (timing.matches()) {
      for (int i = 0; i < NRO_QUOTES; i++) {
        fillQuote(i);
      }
    }
  }

  void fillQuote(int i) {
    ParamStream httpBodyResponse;
    log(CLASS_QUOTES, Debug, "Filling %d", i);
    int errorCode = httpGet(URL_QUOTES, &httpBodyResponse);
    if (errorCode == HTTP_OK) {
      quotes[i]->fill("%s", httpBodyResponse.content());
    } else {
      log(CLASS_QUOTES, Warn, "KO: %d", errorCode);
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {}

  int getNroProps() {
    return 0;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &timing;
  }

  const char *getQuote(int i) {
    int vi = POSIT(i % NRO_QUOTES);
    return quotes[vi]->getBuffer();
  }
};

#endif // QUOTES_INC
