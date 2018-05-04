#ifndef SETUPSYNC_INC
#define SETUPSYNC_INC

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

#define CLASS_SETUPSYNC "SS"
#define MAX_URL_EFF_LENGTH 100

#define WAIT_BEFORE_REPOST_DWEETIO_MS 1500
#define DWEET_IO_API_URL_BASE "http://dweet.io"
#define DWEET_IO_API_URL_BASE_POST DWEET_IO_API_URL_BASE "/dweet/for/%s"                  // device
#define DWEET_IO_API_URL_BASE_GET DWEET_IO_API_URL_BASE "/get/latest/dweet/for/%s-target" // device

#ifndef DWEET_IO_API_TOKEN
#error "Must provide DWEET_IO_API_TOKEN"
#endif

/**
* This actor exchanges status via HTTP.
* TODO: document, reuse PropSync
*/
class SetupSync : public HttpJsonActor {

private:
  const char *name;
  WebBot *bot;
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  Buffer<MAX_JSON_STR_LENGTH> staticBuffer;
  Buffer<MAX_URL_EFF_LENGTH> staticUrl;
  wl_status_t (*initWifiFunc)();

public:
  SetupSync(const char *n) : freqConf(OnceEvery1Minute) {
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
      log(CLASS_SETUPSYNC, Error, "Init needed");
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

  void setUpDweetClient(HTTPClient *client, Buffer<MAX_URL_EFF_LENGTH> *url) {
    client->begin(url->getBuffer());
    client->addHeader("Content-Type", "application/json");
    client->addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);
    log(CLASS_SETUPSYNC, Info, "Connected DWT: %s", url->getBuffer());
  }

  void updateBotProperties() {
#ifndef UNIT_TEST
    int errorCode;


    log(CLASS_SETUPSYNC, Debug, "Setup encryption");
    struct AES_ctx ctx;
    uint8_t key[16 + 1] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c, 0 };
    uint8_t inOut[16 + 1]  = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a, 0 };

    AES_init_ctx(&ctx, key);

    log(CLASS_SETUPSYNC, Debug, "Non encrypted: %s", inOut);
    AES_ECB_encrypt(&ctx, inOut);
    log(CLASS_SETUPSYNC, Debug, "Encrypted: %s", inOut);

    log(CLASS_MAIN, Debug, "Write to EEPROM: %s", inOut);
    for (int i = 0; i < 16; i++) {
      EEPROM.write(i, inOut[i]);
    }



    delay(WAIT_BEFORE_REPOST_DWEETIO_MS);

    HTTPClient client;

    ParamStream s;
    staticUrl.clear();
    staticUrl.fill(DWEET_IO_API_URL_BASE_GET, DEVICE_NAME);
    setUpDweetClient(&client, &staticUrl);
    errorCode = client.GET();
    log(CLASS_SETUPSYNC, Info, "HTTP GET DWT: %d", errorCode);
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
          log(CLASS_SETUPSYNC, Warn, "No 'content'");
        }
      } else {
        log(CLASS_SETUPSYNC, Warn, "No 'with'");
      }
      s.flush();
    } else {
      log(CLASS_SETUPSYNC, Error, "! %s", client.errorToString(errorCode).c_str());
    }

    bot->getPropsJsonFlat(&staticBuffer);
    staticUrl.clear();
    staticUrl.fill(DWEET_IO_API_URL_BASE_POST, DEVICE_NAME);
    log(CLASS_SETUPSYNC, Info, "HTTP POST DWT: %s", staticBuffer.getBuffer());

    setUpDweetClient(&client, &staticUrl);
    errorCode = client.POST(staticBuffer.getBuffer());
    log(CLASS_SETUPSYNC, Info, "HTT POST DWT: %d", errorCode);
    if (errorCode > 0) {
      client.writeToStream(&Serial);
      client.end();
      delay(WAIT_BEFORE_REPOST_DWEETIO_MS);
    } else {
      log(CLASS_SETUPSYNC, Error, "! %s", client.errorToString(errorCode).c_str());
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

#endif // SETUPSYNC_INC
