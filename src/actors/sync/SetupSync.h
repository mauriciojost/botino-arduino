#ifndef SETUPSYNC_INC
#define SETUPSYNC_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Misc.h>
#include <main4ino/Bot.h>
#include <actors/sync/ParamStream.h>
#include <main4ino/Clock.h>
#include <main4ino/Boolean.h>
#include <Hexer.h>
#ifdef UNIT_TEST
#include <aes.h> // in C code
#else
#include <aes.hpp> // in C++ code
#endif

#define CLASS_SETUPSYNC "SS"

#define DWEET_IO_API_URL_BASE_GET "http://dweet.io/get/latest/dweet/for/" DEVICE_NAME "-setup"

#define KEY_LENGTH 16 // AES128

#define N_BLOCKS 2 // 2 times KEY_LENGTH

#define ENCRYPTION_BUFFER_SIZE (N_BLOCKS * KEY_LENGTH + 1) // encryption zone + trailing null character

/**
* This actor performs WIFI setup via HTTP.
*/
class SetupSync : public Actor {

private:
  const char *name;

  char ssid[ENCRYPTION_BUFFER_SIZE];
  char pass[ENCRYPTION_BUFFER_SIZE];
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  bool (*initWifiSteadyFunc)();
  bool (*initWifiInitFunc)();
  int (*httpGet)(const char* url, ParamStream* response);

  struct AES_ctx ctx;
  uint8_t key[KEY_LENGTH];

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
              char passEncHex[ENCRYPTION_BUFFER_SIZE * 2]; // each character (including null ending trail) is represented using 2 chars

              // SSID recovery
              const char* s = content["ssid"].as<char *>();
              strcpy(ssid, s);

              // PASS recovery (encrypted and hex encoded)
              const char* p = content["pass"].as<char *>();
              strcpy(passEncHex, p);
              Hexer::hexStrCpy((uint8_t*)pass, passEncHex);
              decrypt((uint8_t*)pass);

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

  void encrypt(const uint8_t* buffer) {
    log(CLASS_SETUPSYNC, Debug, "Original: %s", buffer);
    for (int i = 0; i < N_BLOCKS; ++i) {
    	const uint8_t* bufferBlock = buffer + (i * KEY_LENGTH);
      phex("Original buffer", bufferBlock);
      AES_ECB_encrypt(&ctx, bufferBlock);
      phex("Encrypted buffer", bufferBlock);
    }
  }

  void decrypt(const uint8_t* buffer) {
    for (int i = 0; i < N_BLOCKS; ++i) {
    	const uint8_t* bufferBlock = buffer + (i * KEY_LENGTH);
      phex("Encrypted buffer", bufferBlock);
      AES_ECB_decrypt(&ctx, bufferBlock);
      phex("Decrypted buffer", bufferBlock);
    }
    log(CLASS_SETUPSYNC, Debug, "Decrypted: %s", buffer);
  }

  // Prints string as hex
  void phex(const char* name, const uint8_t* str) {
    uint8_t len = KEY_LENGTH;
    log(CLASS_SETUPSYNC, Debug, "%s contains:", name);
    for (int i = 0; i < len; ++i) {
      log(CLASS_SETUPSYNC, Debug, " %.2x", str[i]);
    }
  }


public:

  SetupSync(const char *n) : freqConf(OnceEvery5Minutes) {
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
