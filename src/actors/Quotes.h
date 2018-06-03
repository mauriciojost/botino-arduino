#ifndef QUOTES_INC
#define QUOTES_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>
#include <main4ino/Misc.h>

#define CLASS_QUOTES "QU"

#define QUOTE_MAX_LENGTH 128
#define NRO_QUOTES 6

enum QuotesConfigState {
  QuotesConfigStateDelimiter = 0 // delimiter of the configuration states
};

#define URL_QUOTES "http://api.forismatic.com/api/1.0/POST?method=getQuote&format=text&lang=en"

/**
 * Retrieve quotes from the internet.
 */
class Quotes : public Actor {

private:
  const char *name;
  Timing timing;
  Buffer<QUOTE_MAX_LENGTH> *quotes[NRO_QUOTES];
  int (*httpGet)(const char* url, ParamStream* response);
  bool (*initWifiFunc)();

  bool isInitialized() {
  	return (httpGet != NULL && initWifiFunc != NULL );
  }

public:
  Quotes(const char *n) : timing(OnceEvery5Minutes) {
    name = n;
    httpGet = NULL;
    initWifiFunc = NULL;
    for (int i = 0; i < NRO_QUOTES; i++) {
      quotes[i] = new Buffer<QUOTE_MAX_LENGTH>("Damn! No quote yet! :(");
    }
  }

  const char *getName() {
    return name;
  }

  void setHttpGet(int (*h)(const char* url, ParamStream* response)) {
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
    if (errorCode > 0) {
      quotes[i]->fill("%s", httpBodyResponse.content());
    } else {
      log(CLASS_QUOTES, Warn, "No quote");
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      default:
        return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value *targetValue, Value *actualValue) { }

  int getNroProps() {
    return QuotesConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &timing;
  }

  const char* getQuote(int i) {
  	int vi = POSIT(i % NRO_QUOTES);
  	return quotes[vi]->getBuffer();
  }

};

#endif // QUOTES_INC