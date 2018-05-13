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

#define BUFF_SIZE 32



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
  unsigned long long int myIv; // CBC initialization vector; real iv = iv x2 ex: 01234567 = 0123456701234567


  AES aes ;

public:
  SetupSync(const char *n) : freqConf(OnceEvery1Minute) {
    name = n;
    initWifiSteadyFunc = NULL;
    initWifiInitFunc = NULL;
    ssid[0] = 0;
    pass[0] = 0;
    httpGet = NULL;
    myIv = 36753562;
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

  int value(char v) {
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

  void hexstrcpy(char* outputText, const char* inputHex) {
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
              const char* s = content["ssid"].as<char *>();
              const char* p = content["pass"].as<char *>();
              strcpy(ssid, s);
              strcpy(pass, p);

              const char* input = "http://www.arduinolab.net/aypt";
              const char* plainKey = "01234567890123456789012345678901";

              int plainPaddedLength = BUFF_SIZE  + (N_BLOCK - ((BUFF_SIZE - 1) % 16)); // length of padded plaintext [B]
              char encrypted [plainPaddedLength];
              char decrypted [plainPaddedLength];

              encrypt(input, plainKey, encrypted);
              decrypt(encrypted, plainKey, decrypted);

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

  void encrypt(const char* plainInput, const char* plainKey, char* encrypted) {
    byte iv [N_BLOCK] ;
    aes.iv_inc();
    aes.set_IV(myIv);
    aes.get_IV(iv);
    aes.do_aes_encrypt((byte*)plainInput,BUFF_SIZE + 1,(byte*)encrypted,(byte*)plainKey,KEY_LENGTH,iv);
    log(CLASS_SETUPSYNC, Debug, "Original: %s", plainInput);
    log(CLASS_SETUPSYNC, Debug, "Encrypted: %s", encrypted);
  }

  void decrypt(const char* encrypted, const char* plainKey, char* decrypted) {
    byte iv [N_BLOCK] ;
    aes.iv_inc();
    aes.set_IV(myIv);
    aes.get_IV(iv);
    aes.do_aes_decrypt((byte*)encrypted,aes.get_size(),(byte*)decrypted,(byte*)plainKey,KEY_LENGTH,iv);
    log(CLASS_SETUPSYNC, Debug, "Encrypted: %s", encrypted);
    log(CLASS_SETUPSYNC, Debug, "Decrypted: %s", decrypted);
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
