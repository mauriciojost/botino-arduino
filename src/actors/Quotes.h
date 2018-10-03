#ifndef QUOTES_INC
#define QUOTES_INC

#include <main4ino/HttpCodes.h>
#include <main4ino/ParamStream.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Misc.h>

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
  Metadata* md;
  Buffer *quotes[NRO_QUOTES];
  int (*httpGet)(const char *url, ParamStream *response, Table* headers);
  Buffer* jsonAuxBuffer;
  bool (*initWifiFunc)();
  Table* headers;

  bool isInitialized() {
    return (httpGet != NULL && initWifiFunc != NULL);
  }

public:
  Quotes(const char *n) {
    name = n;
    httpGet = NULL;
    initWifiFunc = NULL;
    md = new Metadata(n);

    jsonAuxBuffer = new Buffer(MAX_JSON_STR_LENGTH);
    md->getTiming()->setFrek(201126060);
    for (int i = 0; i < NRO_QUOTES; i++) {
      quotes[i] = new Buffer(QUOTE_MAX_LENGTH, "Damn! No quote yet! :(");
    }
    headers = new Table(0, 0, 0);
  }

  const char *getName() {
    return name;
  }

  void setHttpGet(int (*h)(const char *url, ParamStream *response, Table* headers)) {
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
    if (getTiming()->matches()) {
      for (int i = 0; i < NRO_QUOTES; i++) {
        fillQuote(i);
      }
    }
  }

  void fillQuote(int i) {
    ParamStream httpBodyResponse(jsonAuxBuffer);
    log(CLASS_QUOTES, Debug, "Filling %d", i);
    int errorCode = httpGet(URL_QUOTES, &httpBodyResponse, headers);
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

  void getInfo(int infoIndex, Buffer *info) {}

  int getNroInfos() {
    return 0;
  }

  Metadata *getMetadata() {
    return md;
  }

  const char *getQuote(int i) {
    int vi = POSIT(i % NRO_QUOTES);
    return quotes[vi]->getBuffer();
  }
};

#endif // QUOTES_INC
