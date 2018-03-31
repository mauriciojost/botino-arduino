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

public:
  Messenger(const char *n) : freqConf(OnceEvery10Seconds) {
    name = n;
    bot = NULL;
  }

  void setBot(WebBot *b) {
    bot = b;
  }

  const char *getName() {
    return name;
  }

  void cycle() {
    if (bot == NULL) {
      return;
    }
    if (freqConf.matches()) {
      updateClockProperties();
      updateBotProperties();
    }
  }

  void updateClockProperties() {
#ifndef UNIT_TEST
    ParamStream s;
    int errorCode;

    HTTPClient client;
    staticBuffer.clear();
    staticBuffer.fill(TIMEZONE_DB_API_URL_BASE_GET, TIMEZONE_DB_KEY, TIMEZONE_DB_ZONE);
    client.begin(staticBuffer.getBuffer());
    log(CLASS_MESSENGER, Info, "Url clk: %s", staticBuffer.getBuffer());
    errorCode = client.GET();
    log(CLASS_MESSENGER, Info, "Get clk: %d", errorCode);
    if (errorCode > 0) {
      client.writeToStream(&s);
      client.end();

      JsonObject &json = s.parse();

      if (json.containsKey("formatted")) {

        Boolean autoAdjust(true);
        bot->getClock()->setProp(ClockConfigStateAutoAdjustFactor, SetValue, &autoAdjust, NULL);

        const char *formatted = json["formatted"].as<char *>();
        Buffer<8> time(formatted + 11);
        bot->getClock()->setProp(ClockConfigStateHhMmSs, SetValue, &time, NULL);

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
    log(CLASS_MESSENGER, Info, "Connected dwt %s", url->getBuffer());
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
    log(CLASS_MESSENGER, Info, "Get dwt: %d", errorCode);
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
    log(CLASS_MESSENGER, Info, "Post dwt: %s", staticBuffer.getBuffer());

    setUpDweetClient(&client, &staticUrl);
    errorCode = client.POST(staticBuffer.getBuffer());
    log(CLASS_MESSENGER, Info, "Pos dwt: %d", errorCode);
    if (errorCode > 0) {
      client.writeToStream(&Serial);
      client.end();
      delay(WAIT_BEFORE_REPOST_DWEETIO_MS);
    } else {
      log(CLASS_MESSENGER, Error, "! %s", client.errorToString(errorCode).c_str());
    }

#endif // UNIT_TEST
  }

  void getActuatorValue(Value *value) {}

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
