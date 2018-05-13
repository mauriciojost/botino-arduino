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
    //myIv = 36753562;
    myIv = 1;
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

  void hexstrcpy(byte* outputText, const char* inputHex) {
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

              const byte* plainKey = (byte*)"0000000000000000";

              int plainPaddedLength = BUFF_SIZE  + (N_BLOCK - ((BUFF_SIZE - 1) % 16)); // length of padded plaintext [B]

              char passEncHex[plainPaddedLength * 2];
              byte passEnc[plainPaddedLength];

              const char* s = content["ssid"].as<char *>();
              const char* p = content["pass"].as<char *>();

              log(CLASS_SETUPSYNC, Warn, "KKK %s", p);
              strcpy(ssid, s);
              strcpy(passEncHex, p);

              hexstrcpy(passEnc, passEncHex);
              log(CLASS_SETUPSYNC, Warn, "KKK %s", passEnc);

              decrypt(passEnc, plainKey, (byte*)pass);

              byte encrypted[plainPaddedLength];
              byte decrypted[plainPaddedLength];
              encrypt((byte*)"00", plainKey, encrypted);
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

  void encrypt(const byte* plainInput, const byte* plainKey, byte* encrypted) {
    byte iv [N_BLOCK] ;
    aes.iv_inc();
    aes.set_IV(myIv);
    aes.get_IV(iv);
    log(CLASS_SETUPSYNC, Debug, "IV: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", iv[0], iv[1], iv[2], iv[3], iv[4], iv[5], iv[6], iv[7], iv[8], iv[9], iv[10], iv[11], iv[12], iv[13], iv[14], iv[15]);
    aes.do_aes_encrypt((byte*)plainInput,BUFF_SIZE + 1,(byte*)encrypted,(byte*)plainKey,KEY_LENGTH,iv);
    log(CLASS_SETUPSYNC, Debug, "Original: %s", plainInput);
    log(CLASS_SETUPSYNC, Debug, "Encrypted: %s", encrypted);
    log(CLASS_SETUPSYNC, Debug, "Encrypted: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", encrypted[0], encrypted[1], encrypted[2], encrypted[3], encrypted[4], encrypted[5], encrypted[6], encrypted[7], encrypted[8], encrypted[9], encrypted[10], encrypted[11], encrypted[12], encrypted[13], encrypted[14], encrypted[15]);
  }

  void decrypt(const byte* encrypted, const byte* plainKey, byte* decrypted) {
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
