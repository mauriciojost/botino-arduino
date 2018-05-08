#ifndef SETUPSYNC_INC
#define SETUPSYNC_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Misc.h>
#include <main4ino/Bot.h>
#include <actors/sync/ParamStream.h>
#include <main4ino/Clock.h>
#include <main4ino/Boolean.h>
//#include <aes.hpp>

#define CLASS_SETUPSYNC "SS"

#define DWEET_IO_API_URL_BASE_GET "http://dweet.io/get/latest/dweet/for/" DEVICE_NAME "-setup"

/**
* This actor performs WIFI setup via HTTP.
*/
class SetupSync : public Actor {

private:
  const char *name;

  const uint8_t* key = (uint8_t*)"1234567890123456"; // length 16
  char ssid[16];
  char pass[16];
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  wl_status_t (*initWifiSteadyFunc)();
  wl_status_t (*initWifiInitFunc)();
  int (*httpGet)(const char* url, ParamStream* response);

public:
  SetupSync(const char *n) : freqConf(OnceEvery1Minute) {
    name = n;
    initWifiSteadyFunc = NULL;
    initWifiInitFunc = NULL;
    ssid[0] = 'X';
    ssid[1] = 0;
    pass[0] = 0;
    httpGet = NULL;
  }

  const char *getName() {
    return name;
  }

  void act() {
    if (initWifiSteadyFunc == NULL || initWifiInitFunc == NULL || httpGet == NULL) {
      log(CLASS_SETUPSYNC, Error, "Init needed");
      return;
    }
    if (freqConf.matches()) {
      update();
    }
  }

  void setInitWifiSteady(wl_status_t (*f)()) {
    initWifiSteadyFunc = f;
  }

  void setInitWifiInit(wl_status_t (*f)()) {
    initWifiInitFunc = f;
  }

  void setHttpGet(int (*h)(const char* url, ParamStream* response)) {
  	httpGet = h;
  }

  void update() {
    wl_status_t status = initWifiSteadyFunc();
    if (status == WL_CONNECTED) {
    	return; // nothing to be done, as already connected
    }

    // try connecting to alternative network and setup
    status = initWifiInitFunc();

    if (status == WL_CONNECTED) {
      ParamStream s;
      int errorCode = httpGet(DWEET_IO_API_URL_BASE_GET, &s);
      if (errorCode > 0) {
        JsonObject &json = s.parse();
        if (json.containsKey("with")) {
          JsonObject &withJson = json["with"][0];
          if (withJson.containsKey("content")) {
            JsonObject &content = withJson["content"];
            if (content.containsKey("ssid")) {
              const char* s = content["ssid"].as<char *>();
              const char* p = content["pass"].as<char *>();
              //struct AES_ctx ctx;
              //AES_init_ctx(&ctx, key);
              strcpy(ssid, s);
              strcpy(pass, p);
              //AES_ECB_decrypt(&ctx, (const unsigned char*)pass);
              log(CLASS_SETUPSYNC, Debug, "Got setup: %s / %s (%s)", s, p, pass);
            } else {
              log(CLASS_SETUPSYNC, Warn, "No 'ssid'");
            }
          } else {
            log(CLASS_SETUPSYNC, Warn, "No 'content'");
          }
        } else {
          log(CLASS_SETUPSYNC, Warn, "No 'with'");
        }
      }
    }
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
