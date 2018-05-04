#ifndef PROPSYNC_INC
#define PROPSYNC_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/WebBot.h>
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

#define WAIT_BEFORE_REPOST_DWEETIO_MS 1500
#define DWEET_IO_API_URL_POST "http://dweet.io/dweet/for/" DEVICE_NAME
#define DWEET_IO_API_URL_GET "http://dweet.io/get/latest/dweet/for/" DEVICE_NAME "-target"

#ifndef DWEET_IO_API_TOKEN
#error "Must provide DWEET_IO_API_TOKEN"
#endif

/**
* This actor exchanges status via HTTP to syncronize
* properties with target property values provided by an user
* on a centralized server, polled regularly.
*/
class PropSync : public Actor {

private:
  const char *name;
  WebBot *bot;
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  Buffer<MAX_JSON_STR_LENGTH> staticBuffer;
  wl_status_t (*initWifiFunc)();

public:
  PropSync(const char *n) : freqConf(OnceEvery1Minute) {
    name = n;
    bot = NULL;
    initWifiFunc = NULL;
  }

  void setBot(WebBot *b) {
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
        updateBotProperties();
    	}
    }
  }
  void setInitWifi(wl_status_t (*f)()) {
    initWifiFunc = f;
  }

  void setUpDweetClient(HTTPClient *client, const char *url) {
    client->begin(url);
    client->addHeader("Content-Type", "application/json");
    client->addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);
    log(CLASS_PROPSYNC, Info, "Connected DWT: %s", url);
  }

  void updateBotProperties() {
#ifndef UNIT_TEST
    int errorCode;

    delay(WAIT_BEFORE_REPOST_DWEETIO_MS);

    HTTPClient client;

    ParamStream s;
    setUpDweetClient(&client, DWEET_IO_API_URL_GET);
    errorCode = client.GET();
    log(CLASS_PROPSYNC, Info, "HTTP GET DWT: %d", errorCode);
    if (errorCode > 0) {
      client.writeToStream(&s);
      client.end();

      JsonObject &json = s.parse();

      if (json.containsKey("with")) {
        JsonObject &withJson = json["with"][0];
        if (withJson.containsKey("content")) {
          JsonObject &content = withJson["content"];
          bot->setPropsJsonFlat(content);
        } else {
          log(CLASS_PROPSYNC, Warn, "No 'content'");
        }
      } else {
        log(CLASS_PROPSYNC, Warn, "No 'with'");
      }
      s.flush();
    } else {
      log(CLASS_PROPSYNC, Error, "! %s", client.errorToString(errorCode).c_str());
    }

    bot->getPropsJsonFlat(&staticBuffer);
    log(CLASS_PROPSYNC, Info, "HTTP POST DWT: %s", DWEET_IO_API_URL_POST);

    setUpDweetClient(&client, DWEET_IO_API_URL_POST);
    errorCode = client.POST(staticBuffer.getBuffer());
    log(CLASS_PROPSYNC, Info, "HTT POST DWT: %d", errorCode);
    if (errorCode > 0) {
      client.writeToStream(&Serial);
      client.end();
      delay(WAIT_BEFORE_REPOST_DWEETIO_MS);
    } else {
      log(CLASS_PROPSYNC, Error, "! %s", client.errorToString(errorCode).c_str());
    }

#endif // UNIT_TEST
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
