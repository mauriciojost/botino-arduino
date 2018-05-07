#ifndef PROPSYNC_INC
#define PROPSYNC_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/SerBot.h>
#include <main4ino/Misc.h>
#ifndef UNIT_TEST
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif // UNIT_TEST
#include <main4ino/Bot.h>
#include <actors/sync/ParamStream.h>
#include <main4ino/Clock.h>
#include <main4ino/Boolean.h>

#define CLASS_PROPSYNC "PS"

#define WAIT_BEFORE_HTTP_DWEETIO_MS 1500
#define DWEET_IO_API_URL_POST "http://dweet.io/dweet/for/" DEVICE_NAME "-%s-current"
#define DWEET_IO_API_URL_GET "http://dweet.io/get/latest/dweet/for/" DEVICE_NAME "-%s-target"

#ifndef DWEET_IO_API_TOKEN
#error "Must provide DWEET_IO_API_TOKEN"
#endif

/**
* This actor exchanges status via HTTP to synchronize
* properties with target property values provided by an user
* on a centralized server, polled regularly.
*/
class PropSync : public Actor {

private:
  const char *name;
  SerBot *bot;
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  Buffer<128> urlAuxBuffer;
  Buffer<MAX_JSON_STR_LENGTH> jsonAuxBuffer;
  ParamStream httpBodyResponse;
  wl_status_t (*initWifiFunc)();

public:
  PropSync(const char *n) : freqConf(OnceEvery1Minute) {
    name = n;
    bot = NULL;
    initWifiFunc = NULL;
  }

  void setBot(SerBot *b) {
    bot = b;
  }

  const char *getName() {
    return name;
  }

  void act() {
    if (bot == NULL || initWifiFunc == NULL) {
      log(CLASS_PROPSYNC, Error, "Init needed");
      return;
    }
    if (freqConf.matches()) {
    	wl_status_t wifiStatus = initWifiFunc();
    	if (wifiStatus == WL_CONNECTED) {
    		for (int i = 0; i < bot->getActors()->size(); i++) {
          updateProps(i);
    		}
    	}
    }
  }
  void setInitWifi(wl_status_t (*f)()) {
    initWifiFunc = f;
  }

  int httpGet(const char* url, ParamStream* response) {
    HTTPClient client;
    client.begin(url);
    client.addHeader("Content-Type", "application/json");
    client.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);

    int errorCode = client.GET();
    log(CLASS_PROPSYNC, Info, "HTTP GET: %s %d", url, errorCode);

    if (errorCode > 0) {
    	response->flush();
      client.writeToStream(response);
    } else {
      log(CLASS_PROPSYNC, Error, "! %s", client.errorToString(errorCode).c_str());
    }
    client.end();

    return errorCode;
  }

  int httpPost(const char* url, const char* body, ParamStream* response) {
    HTTPClient client;
    client.begin(url);
    client.addHeader("Content-Type", "application/json");
    client.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);

    int errorCode = client.POST(body);
    log(CLASS_PROPSYNC, Info, "HTT POST: %s %d", url, errorCode);

    if (errorCode > 0) {
    	response->flush();
      client.writeToStream(response);
    } else {
      log(CLASS_PROPSYNC, Error, "! %s", client.errorToString(errorCode).c_str());
    }
    client.end();

    return errorCode;
  }

  void updateProps(int actorIndex) {
    Actor* actor = bot->getActors()->get(actorIndex);

    delay(WAIT_BEFORE_HTTP_DWEETIO_MS);

    urlAuxBuffer.fill(DWEET_IO_API_URL_GET, actor->getName());
    int errorCode = httpGet(urlAuxBuffer.getBuffer(), &httpBodyResponse);
    if (errorCode > 0) {
      JsonObject &json = httpBodyResponse.parse();
      if (json.containsKey("with")) {
        JsonObject &withJson = json["with"][0];
        if (withJson.containsKey("content")) {
          JsonObject &content = withJson["content"];
          bot->setPropsJson(content, actorIndex);
        } else {
          log(CLASS_PROPSYNC, Warn, "No 'content'");
        }
      } else {
        log(CLASS_PROPSYNC, Warn, "No 'with'");
      }
    }

    delay(WAIT_BEFORE_HTTP_DWEETIO_MS);

    bot->getPropsJson(&jsonAuxBuffer, actorIndex);
    urlAuxBuffer.fill(DWEET_IO_API_URL_POST, actor->getName());
    httpPost(urlAuxBuffer.getBuffer(), jsonAuxBuffer.getBuffer(), &httpBodyResponse); // best effort

  }

  void setProp(int propIndex, SetMode set, const Value *targetValue, Value *actualValue) {}

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
    return &freqConf;
  }
};

#endif // PROPSYNC_INC
