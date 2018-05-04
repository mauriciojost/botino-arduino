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
#include <actors/sync/ParamStream.h>
#include <main4ino/Clock.h>
#include <main4ino/Boolean.h>
#include <aes.hpp>

#define CLASS_SETUPSYNC "SS"

#define DWEET_IO_API_URL_BASE_GET "http://dweet.io/get/latest/dweet/for/" DEVICE_NAME "-setup"

#ifndef DWEET_IO_API_TOKEN
#error "Must provide DWEET_IO_API_TOKEN"
#endif

/**
* This actor performs WIFI setup via HTTP.
*/
class SetupSync : public Actor {

private:
  const char *name;
  char ssid[16];
  char pass[16];
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  wl_status_t (*initWifiFunc)();
  wl_status_t (*initWifiOriginFunc)();

public:
  SetupSync(const char *n) : freqConf(OnceEvery1Minute) {
    name = n;
    initWifiFunc = NULL;
    initWifiOriginFunc = NULL;
    ssid[0] = 0;
    pass[0] = 0;
  }

  const char *getName() {
    return name;
  }

  void act() {
    if (initWifiFunc == NULL || initWifiOriginFunc == NULL) {
      log(CLASS_SETUPSYNC, Error, "Init needed");
      return;
    }
    if (freqConf.matches()) {
      update();
    }
  }

  void setInitWifi(wl_status_t (*f)()) {
    initWifiFunc = f;
  }

  void setInitWifiOrigin(wl_status_t (*f)()) {
    initWifiOriginFunc = f;
  }

  void update() {
#ifndef UNIT_TEST
    int errorCode;

    wl_status_t status = initWifiFunc();
    if (status == WL_CONNECTED) {
    	return; // nothing to be done, as already connected
    }

    // try connecting to alternative network and setup
    status = initWifiOriginFunc();

    if (status == WL_CONNECTED) {

      HTTPClient client;
      ParamStream s;

      client.begin(DWEET_IO_API_URL_BASE_GET);
      client.addHeader("Content-Type", "application/json");
      client.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);
      log(CLASS_SETUPSYNC, Info, "Connected DWT: %s", DWEET_IO_API_URL_BASE_GET);

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
            if (content.containsKey("ssid")) {
              const char* s = content["ssid"].as<char *>();
              const char* p = content["pass"].as<char *>();
              strcpy(ssid, s);
              strcpy(pass, p);

              uint8_t key[16 + 1] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c, 0 };
              log(CLASS_SETUPSYNC, Debug, "Setup encryption");
              struct AES_ctx ctx;
              AES_init_ctx(&ctx, key);
              AES_ECB_decrypt(&ctx, (const unsigned char*)pass);

            } else {
              log(CLASS_SETUPSYNC, Warn, "No 'ssid'");
            }
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

  const char* getSsid() {
  	return ssid;
  }

  const char* getPass() {
  	return pass;
  }

  Timing *getFrequencyConfiguration() {
    return &freqConf;
  }
};

#endif // SETUPSYNC_INC
