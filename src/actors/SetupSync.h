#ifndef SETUPSYNC_INC
#define SETUPSYNC_INC

#include <log4ino/Log.h>
#include <Hexer.h>
#include <HttpCodes.h>
#include <main4ino/Actor.h>
#include <actors/ParamStream.h>
#include <main4ino/Boolean.h>
#include <main4ino/Bot.h>
#include <main4ino/Clock.h>
#include <main4ino/Misc.h>
#include <main4ino/Metadata.h>

#define CLASS_SETUPSYNC "SS"

#ifndef WIFI_SSID_STEADY
#define WIFI_SSID_STEADY "???"
#endif // WIFI_SSID_STEADY

#ifndef WIFI_PASSWORD_STEADY
#define WIFI_PASSWORD_STEADY "???"
#endif // WIFI_PASSWORD_STEADY

#ifndef IFTTT_KEY
#define IFTTT_KEY "???"
#endif // IFTTT_KEY

#ifndef TIMEZONE_DB_KEY
#define TIMEZONE_DB_KEY "???"
#endif

#define CREDENTIAL_BUFFER_SIZE 64

enum SetupSyncProps {
  SetupSyncWifiSsidProp = 0, // wifi ssid
  SetupSyncWifiPassProp, // wifi pass
  SetupSyncIftttKeyProp, // ifttt key (webhook)
  SetupSyncITimeKeyProp, // time api key
  SetupSyncPropsDelimiter // delimiter of the configuration states
};

/**
 * This actor holds the setup/credentials/sensitive information for the rest of the modules to use.
 */
// TODO credentials are to be stored in EEPROM
// TODO credentials are to be setup either via macros or via serial port
class SetupSync : public Actor {

private:
  const char *name;

  char ssid[CREDENTIAL_BUFFER_SIZE];
  char pass[CREDENTIAL_BUFFER_SIZE];
  char ifttt[CREDENTIAL_BUFFER_SIZE];
  char timeKey[CREDENTIAL_BUFFER_SIZE];
  Metadata* md;

public:
  SetupSync(const char *n) {
    name = n;
    strcpy(ssid, WIFI_SSID_STEADY);
    strcpy(pass, WIFI_PASSWORD_STEADY);
    strcpy(ifttt, IFTTT_KEY);
    strcpy(timeKey, TIMEZONE_DB_KEY);
    md = new Metadata(n);
  }

  const char *getName() {
    return name;
  }

  void act() {}

  void getSetPropValue(int propIndex, GetSetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
    	case (SetupSyncWifiSsidProp):
        setPropValue(setMode, targetValue, actualValue, &ssid);
        break;
    	case (SetupSyncWifiPassProp):
        setPropValue(setMode, targetValue, actualValue, &pass);
        break;
    	case (SetupSyncIftttKeyProp):
        setPropValue(setMode, targetValue, actualValue, &ifttt);
        break;
    	case (SetupSyncITimeKeyProp):
        setPropValue(setMode, targetValue, actualValue, &timeKey);
        break;
      default:
        break;
    }
    if (setMode != GetValue) {
    	getMetadata()->changed();
    }
  }

  int getNroProps() {
    return 4;
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
    	case (SetupSyncWifiSsidProp):
    			return "_wifissid";
    	case (SetupSyncWifiPassProp):
    			return "_wifipass";
    	case (SetupSyncIftttKeyProp):
        return "_iftttkey";
    	case (SetupSyncITimeKeyProp):
        return "_timekey";
      default:
        return "";
    }
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  const char *getSsid() {
    return ssid;
  }

  void setSsid(const char *s) {
    strcpy(ssid, s);
  }

  const char *getPass() {
    return pass;
  }

  void setPass(const char *s) {
    strcpy(pass, s);
  }

  const char *getIfttt() {
    return ifttt;
  }

  void setIfttt(const char *s) {
    strcpy(ifttt, s);
  }

  const char *getTimeKey() {
    return timeKey;
  }

  void setTimeKey(const char *s) {
    strcpy(timeKey, s);
  }


  Metadata *getMetadata() {
    return md;
  }

  bool isInitialized() {
    return ssid[0] != '?' && pass[0] != '?' && ifttt[0] != '?';
  }
};

#endif // SETUPSYNC_INC
