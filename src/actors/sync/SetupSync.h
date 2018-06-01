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

#ifndef ENCRYPT_KEY
#error "Must define ENCRYPT_KEY"
#endif // ENCRYPT_KEY


#define N_BLOCKS 2 // 2 times KEY_LENGTH

#define ENCRYPTION_BUFFER_SIZE (N_BLOCKS * KEY_LENGTH + 1) // encryption zone + trailing null character

enum SetupSyncConfigState {
  SetupSyncConfigFreq = 0,
  SetupSyncConfigDelimiter // delimiter of the configuration states
};

/**
* This actor performs the setup of sensitive information via HTTP (via encryption).
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
  uint8_t key[KEY_LENGTH]; // represented as a string, so N chars + 1 trailing null char

  void update() {
    bool connected = initWifiSteadyFunc();
    if (connected) {
    	freqConf.setFrequency(Never);
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
            if (content.containsKey("ssid") && content.containsKey("pass")) {
              char aux[ENCRYPTION_BUFFER_SIZE * 2]; // each character (including null ending trail) is represented using 2 chars

              // SSID recovery (encrypted and hex encoded)
              const char* s = content["ssid"].as<char *>();
              strcpy(aux, s);
              Hexer::hexToByte((uint8_t*)ssid, aux, MINIM(strlen(aux), ENCRYPTION_BUFFER_SIZE * 2));
              decrypt((uint8_t*)ssid);
              ssid[ENCRYPTION_BUFFER_SIZE - 1] = 0;

              // PASS recovery (encrypted and hex encoded)
              const char* p = content["pass"].as<char *>();
              strcpy(aux, p);
              Hexer::hexToByte((uint8_t*)pass, aux, MINIM(strlen(aux), ENCRYPTION_BUFFER_SIZE * 2));
              decrypt((uint8_t*)pass);
              pass[ENCRYPTION_BUFFER_SIZE - 1] = 0;

              log(CLASS_SETUPSYNC, Debug, "SETUP:%s/***", ssid);

            } else {
              log(CLASS_SETUPSYNC, Warn, "No 'ssid'");
            }
          } else {
            log(CLASS_SETUPSYNC, Info, "No 'content'");
          }
        } else {
          log(CLASS_SETUPSYNC, Warn, "No 'with'");
        }
      }
    }
  }

  void encrypt(const uint8_t* buffer) {
    log(CLASS_SETUPSYNC, Debug, "Original: ");
    logHex(CLASS_SETUPSYNC, Debug, buffer, ENCRYPTION_BUFFER_SIZE);
    for (int i = 0; i < N_BLOCKS; ++i) {
    	const uint8_t* bufferBlock = buffer + (i * KEY_LENGTH);
      AES_ECB_encrypt(&ctx, bufferBlock);
    }
    log(CLASS_SETUPSYNC, Debug, "Encrypted: ");
    logHex(CLASS_SETUPSYNC, Debug, buffer, ENCRYPTION_BUFFER_SIZE);
  }

  void decrypt(const uint8_t* buffer) {
    log(CLASS_SETUPSYNC, Debug, "Encrypted: ");
    logHex(CLASS_SETUPSYNC, Debug, buffer, ENCRYPTION_BUFFER_SIZE);
    for (int i = 0; i < N_BLOCKS; ++i) {
    	const uint8_t* bufferBlock = buffer + (i * KEY_LENGTH);
      AES_ECB_decrypt(&ctx, bufferBlock);
    }
    log(CLASS_SETUPSYNC, Debug, "Decrypted: ");
    logHex(CLASS_SETUPSYNC, Debug, buffer, ENCRYPTION_BUFFER_SIZE);
  }

public:

  SetupSync(const char *n) {
    name = n;
    initWifiSteadyFunc = NULL;
    initWifiInitFunc = NULL;
    ssid[0] = 0;
    pass[0] = 0;
    httpGet = NULL;
    freqConf.setFrequency(OnceEvery1Minute);
		Hexer::hexToByte(key, ENCRYPT_KEY, KEY_LENGTH * 2);
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

  void setProp(int propIndex, SetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (SetupSyncConfigFreq):
        {
          long freq = freqConf.getCustom();
          setPropLong(setMode, targetValue, actualValue, &freq);
          if (setMode == SetValue) {
            freqConf.setCustom(freq);
            freqConf.setFrequency(Custom);
          }
        }
        break;
      default:
        break;
    }
  }

  int getNroProps() {
    return SetupSyncConfigDelimiter;
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (SetupSyncConfigFreq):
        return "freq";
      default:
        return "";
    }
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

  bool isInitialized() {
  	return ssid[0] != 0 && pass[0] != 0;
  }
};

#endif // SETUPSYNC_INC
