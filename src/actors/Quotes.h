#ifndef QUOTES_INC
#define QUOTES_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/HttpCodes.h>
#include <main4ino/HttpMethods.h>
#include <main4ino/Integer.h>
#include <main4ino/Misc.h>
#include <main4ino/ParamStream.h>
#include <main4ino/Table.h>

#define CLASS_QUOTES "QU"

#define QUOTE_MAX_LENGTH 128
#define NRO_QUOTES 6

#define URL_QUOTES "http://api.forismatic.com/api/1.0/POST?method=getQuote&format=text&lang=en"

enum QuotesProps {
  QuotesFreqProp = 0,  // frequency of synchronization
  QuotesPropsDelimiter // count of properties
};

/**
 * Retrieve quotes from the internet.
 */
class Quotes : public Actor {

private:
  const char *name;
  Metadata *md;
  Buffer *quotes[NRO_QUOTES];
  int (*httpMethod)(HttpMethod m, const char *url, const char *body, ParamStream *response, Table *headers);
  Buffer *jsonAuxBuffer;
  bool (*initWifiFunc)();
  Table *headers;

  bool isInitialized() {
    return (httpMethod != NULL && initWifiFunc != NULL);
  }

public:
  Quotes(const char *n) {
    name = n;
    httpMethod = NULL;
    initWifiFunc = NULL;
    md = new Metadata(n);

    jsonAuxBuffer = new Buffer(MAX_JSON_STR_LENGTH);
    md->getTiming()->setFreq("never");
    for (int i = 0; i < NRO_QUOTES; i++) {
      quotes[i] = new Buffer(QUOTE_MAX_LENGTH, "Damn! No quote yet! :(");
    }
    headers = new Table(0, 0, 0);
  }

  const char *getName() {
    return name;
  }

  void setHttpMethod(int (*h)(HttpMethod m, const char *url, const char *body, ParamStream *response, Table *headers)) {
    httpMethod = h;
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
    ParamStream httpBodyResponse(jsonAuxBuffer, true);
    log(CLASS_QUOTES, Debug, "Filling %d", i);
    int errorCode = httpMethod(HttpGet, URL_QUOTES, NULL, &httpBodyResponse, headers);
    if (errorCode == HTTP_OK) {
      quotes[i]->fill("%s", httpBodyResponse.content());
    } else {
      log(CLASS_QUOTES, Warn, "KO: %d", errorCode);
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (QuotesFreqProp):
        return "freq";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (QuotesFreqProp): {
        setPropTiming(m, targetValue, actualValue, md->getTiming());
      } break;
      default:
        break;
    }
    if (m != GetValue) {
      getMetadata()->changed();
    }
  }

  int getNroProps() {
    return QuotesPropsDelimiter;
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
