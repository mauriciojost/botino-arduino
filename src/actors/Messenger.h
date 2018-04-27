#ifndef MESSENGER_INC
#define MESSENGER_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/WebBot.h>
#include <main4ino/Misc.h>
#ifndef UNIT_TEST
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif // UNIT_TEST
#include <main4ino/Bot.h>
#include <actors/ParamStream.h>
#include <main4ino/Clock.h>
#include <main4ino/Boolean.h>

#define CLASS_MESSENGER "MS"
#define MAX_URL_EFF_LENGTH 100

#define WAIT_BEFORE_REPOST_DWEETIO_MS 1500
#define DWEET_IO_API_URL_BASE "http://dweet.io"
#define DWEET_IO_API_URL_BASE_POST DWEET_IO_API_URL_BASE "/dweet/for/%s"                  // device
#define DWEET_IO_API_URL_BASE_GET DWEET_IO_API_URL_BASE "/get/latest/dweet/for/%s-target" // device
#define TIMEZONE_DB_API_URL_BASE_GET "http://api.timezonedb.com/v2/get-time-zone?key=%s&format=json&by=zone&zone=%s"

#ifndef DWEET_IO_API_TOKEN
#error "Must provide DWEET_IO_API_TOKEN"
#endif
#ifndef TIMEZONE_DB_KEY
#error "Must provide TIMEZONE_DB_KEY"
#endif
#ifndef TIMEZONE_DB_ZONE
#error "Must provide TIMEZONE_DB_ZONE"
#endif

/**
* This actor exchanges status via HTTP.
*/
class Messenger : public Actor {

private:
  const char *name;
  WebBot *bot;
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  Buffer<MAX_JSON_STR_LENGTH> staticBuffer;
  Buffer<MAX_URL_EFF_LENGTH> staticUrl;
  wl_status_t (*initWifiFunc)();

public:
  Messenger(const char *n) : freqConf(OnceEvery1Minute) {
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
      log(CLASS_MESSENGER, Error, "Init needed");
      return;
    }
    if (freqConf.matches()) {
    	wl_status_t wifiStatus = initWifiFunc();
    	if (wifiStatus == WL_CONNECTED) {
        updateClockProperties();
        updateBotProperties();
    	}
    }
  }
  void setInitWifi(wl_status_t (*f)()) {
    initWifiFunc = f;
  }

  void updateClockProperties() {
#ifndef UNIT_TEST
    ParamStream s;
    int errorCode;

    HTTPClient client;
    staticBuffer.clear();
    staticBuffer.fill(TIMEZONE_DB_API_URL_BASE_GET, TIMEZONE_DB_KEY, TIMEZONE_DB_ZONE);
    client.begin(staticBuffer.getBuffer());
    log(CLASS_MESSENGER, Info, "URL clk: %s", staticBuffer.getBuffer());
    errorCode = client.GET();
    log(CLASS_MESSENGER, Info, "HTTP GET clk: %d", errorCode);
    if (errorCode > 0) {
      client.writeToStream(&s);
      client.end();

      JsonObject &json = s.parse();

      if (json.containsKey("formatted")) {

        int y, mo, d, h, m, s;
        const char *formatted = json["formatted"].as<char *>(); // example: 2018-04-26 21:32:30
        log(CLASS_MESSENGER, Debug, "Retrieved: %s", formatted);
        sscanf(formatted, "%04d-%02d-%02d %02d:%02d:%02d", &y, &mo, &d, &h, &m, &s);
        Buffer<8> time(formatted + 11);
        bot->getClock()->setAutoAdjust(true);
        bot->getClock()->set(DONT_CHANGE, h, m, s);
        bot->getClock()->setAutoAdjust(false);
        bot->getClock()->set(DONT_CHANGE, h, m, s);
        log(CLASS_MESSENGER, Info, "Set time: %s", time.getBuffer());
      } else {
        log(CLASS_MESSENGER, Warn, "No 'formatted'");
      }
      s.flush();
    } else {
      log(CLASS_MESSENGER, Error, "! %s", client.errorToString(errorCode).c_str());
    }
#endif // UNIT_TEST
  }

  void setUpDweetClient(HTTPClient *client, Buffer<MAX_URL_EFF_LENGTH> *url) {
    client->begin(url->getBuffer());
    client->addHeader("Content-Type", "application/json");
    client->addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);
    log(CLASS_MESSENGER, Info, "Connected DWT: %s", url->getBuffer());
  }

  void updateBotProperties() {
#ifndef UNIT_TEST
    int errorCode;

    delay(WAIT_BEFORE_REPOST_DWEETIO_MS);

    HTTPClient client;

    ParamStream s;
    staticUrl.clear();
    staticUrl.fill(DWEET_IO_API_URL_BASE_GET, DEVICE_NAME);
    setUpDweetClient(&client, &staticUrl);
    errorCode = client.GET();
    log(CLASS_MESSENGER, Info, "HTTP GET DWT: %d", errorCode);
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
          log(CLASS_MESSENGER, Warn, "No 'content'");
        }
      } else {
        log(CLASS_MESSENGER, Warn, "No 'with'");
      }
      s.flush();
    } else {
      log(CLASS_MESSENGER, Error, "! %s", client.errorToString(errorCode).c_str());
    }

    bot->getPropsJsonFlat(&staticBuffer);
    staticUrl.clear();
    staticUrl.fill(DWEET_IO_API_URL_BASE_POST, DEVICE_NAME);
    log(CLASS_MESSENGER, Info, "HTTP POST DWT: %s", staticBuffer.getBuffer());

    setUpDweetClient(&client, &staticUrl);
    errorCode = client.POST(staticBuffer.getBuffer());
    log(CLASS_MESSENGER, Info, "HTT POST DWT: %d", errorCode);
    if (errorCode > 0) {
      client.writeToStream(&Serial);
      client.end();
      delay(WAIT_BEFORE_REPOST_DWEETIO_MS);
    } else {
      log(CLASS_MESSENGER, Error, "! %s", client.errorToString(errorCode).c_str());
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

#endif // MESSENGER_INC
