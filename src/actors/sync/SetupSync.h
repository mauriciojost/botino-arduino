#ifndef SETUPSYNC_INC
#define SETUPSYNC_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Misc.h>
#include <main4ino/Bot.h>
#include <actors/sync/ParamStream.h>
#include <main4ino/Clock.h>
#include <main4ino/Boolean.h>
#include <AES.h>

#define CLASS_SETUPSYNC "SS"

#define DWEET_IO_API_URL_BASE_GET "http://dweet.io/get/latest/dweet/for/" DEVICE_NAME "-setup"

#define KEY_LENGTH 128

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


  AES aes ;
  byte *key = (unsigned char*)"01234567890123456789012345678901"; // encryption key
  unsigned long long int myIv = 36753562; // CBC initialization vector; real iv = iv x2 ex: 01234567 = 0123456701234567

public:
  SetupSync(const char *n) : freqConf(OnceEvery1Minute) {
    name = n;
    initWifiSteadyFunc = NULL;
    initWifiInitFunc = NULL;
    ssid[0] = 0;
    pass[0] = 0;
    httpGet = NULL;
  }

  const char *getName() {
    return name;
  }

  void act() {

    aesTest();

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

  void aesTest () {

    const char* plain = "http://www.arduinolab.net/ayptiondecryption-using-arduino-uno/"; // plaintext to encrypt
  	int s = strlen(plain) + 1;

    aes.iv_inc();

    byte iv [N_BLOCK] ;
    int plainPaddedLength = s  + (N_BLOCK - ((s - 1) % 16)); // length of padded plaintext [B]
    const char* cipher [plainPaddedLength]; // ciphertext (encrypted plaintext)
    const char* check [plainPaddedLength]; // decrypted plaintext

    aes.set_IV(myIv);
    aes.get_IV(iv);

    aes.do_aes_encrypt((byte*)plain,s + 1,(byte*)cipher,key,KEY_LENGTH,iv);

    aes.set_IV(myIv);
    aes.get_IV(iv);

    aes.do_aes_decrypt((byte*)cipher,aes.get_size(),(byte*)check,key,KEY_LENGTH,iv);
    log(CLASS_SETUPSYNC, Debug, "Original: %s", plain);
    log(CLASS_SETUPSYNC, Debug, "Encrypted: %s", cipher);
    log(CLASS_SETUPSYNC, Debug, "Decrypted: %s", check);


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
