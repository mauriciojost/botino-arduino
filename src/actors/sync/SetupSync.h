#ifndef SETUPSYNC_INC
#define SETUPSYNC_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Misc.h>
#include <main4ino/Bot.h>
#include <actors/sync/ParamStream.h>
#include <main4ino/Clock.h>
#include <main4ino/Boolean.h>
#include <aes.h>

#define CLASS_SETUPSYNC "SS"

#define DWEET_IO_API_URL_BASE_GET "http://dweet.io/get/latest/dweet/for/" DEVICE_NAME "-setup"

#define KEY_LENGTH 16 // AES128

/**
* This actor performs WIFI setup via HTTP.
*/
class SetupSync : public Actor {

private:
  const char *name;

  char ssid[16];
  char pass[16];
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  bool (*initWifiSteadyFunc)();
  bool (*initWifiInitFunc)();
  int (*httpGet)(const char* url, ParamStream* response);

  struct AES_ctx ctx;
  unsigned char key[KEY_LENGTH]; // 15 chars + 1 null character

public:
  SetupSync(const char *n) : freqConf(OnceEvery1Minute) {
    name = n;
    initWifiSteadyFunc = NULL;
    initWifiInitFunc = NULL;
    ssid[0] = 0;
    pass[0] = 0;
    httpGet = NULL;
    for (int i = 0; i < KEY_LENGTH; i++) { // TODO make configurable
      key[i] = i;
    }
    AES_init_ctx(&ctx, key);
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

  void setInitWifiSteady(bool (*f)()) {
    initWifiSteadyFunc = f;
  }

  void setInitWifiInit(bool (*f)()) {
    initWifiInitFunc = f;
  }

  void setHttpGet(int (*h)(const char* url, ParamStream* response)) {
  	httpGet = h;
  }

  unsigned char value(char v) {
  	if (v >= '0' && v <= '9') {
  		return v - '0';
  	} else if (v >= 'a' && v <= 'f') {
  		return v - 'a' + 10;
  	} else if (v >= 'A' && v <= 'F') {
  		return v - 'A' + 10;
  	} else {
  		return 0;
  	}
  }

  void hexstrcpy(unsigned char* outputText, const char* inputHex) {
  	int l = strlen(inputHex);
  	if (l % 2 == 0) {
      int i;
  		for(i = 0; i < l; i = i + 2) {
  			outputText[i / 2] = value(inputHex[i]) * 16 + value(inputHex[i + 1]);
  		}
  		outputText[i / 2] = 0;
  	} else {
  		outputText[0] = 0;
  	}
  }

  void update() {
    bool connected = initWifiSteadyFunc();
    if (connected) {
    	return; // nothing to be done, as already connected
    }

    // try connecting to alternative network and setup
    connected = initWifiInitFunc();

    if (connected) {
      ParamStream s;
      int errorCode = httpGet(DWEET_IO_API_URL_BASE_GET, &s);
      if (errorCode > 0) {
        JsonObject &json = s.parse();
        if (json.containsKey("with")) {
          JsonObject &withJson = json["with"][0];
          if (withJson.containsKey("content")) {
            JsonObject &content = withJson["content"];
            if (content.containsKey("ssid")) {

              char passEncHex[KEY_LENGTH * 2 + 1];

              const char* s = content["ssid"].as<char *>();
              const char* p = content["pass"].as<char *>();

              strcpy(ssid, s);
              strcpy(passEncHex, p);

              hexstrcpy((unsigned char*)pass, passEncHex);

              decrypt((unsigned char*)pass);


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

  void encrypt(const unsigned char* buffer) {
    log(CLASS_SETUPSYNC, Debug, "Original: %s", buffer);
    phex("Original buffer", buffer);
    for (int i = 0; i < 1/*4*/; ++i) {
      AES_ECB_encrypt(&ctx, buffer + (i * KEY_LENGTH));
      phex("Encrypted buffer", buffer + (i * KEY_LENGTH));
    }
    log(CLASS_SETUPSYNC, Debug, "Encrypted: %s", buffer);
  }

  void decrypt(const unsigned char* buffer) {
    log(CLASS_SETUPSYNC, Debug, "Encrypted: %s", buffer);
    phex("Encrypted buffer", buffer);
    AES_ECB_decrypt(&ctx, buffer);
    log(CLASS_SETUPSYNC, Debug, "Decrypted: %s", buffer);
    phex("Decrypted buffer", buffer);
  }

  // Prints string as hex
  void phex(const char* name, const unsigned char* str) {
    uint8_t len = KEY_LENGTH;
    log(CLASS_SETUPSYNC, Debug, "%s contains:", name);
    for (int i = 0; i < len; ++i) {
      log(CLASS_SETUPSYNC, Debug, " %.2x", str[i]);
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
