#ifndef SETUPSYNC_INC
#define SETUPSYNC_INC

#include <Hexer.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Bot.h>
#include <main4ino/Clock.h>
#include <main4ino/HttpCodes.h>
#include <main4ino/Metadata.h>
#include <main4ino/Misc.h>
#include <main4ino/ParamStream.h>

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
#endif // TIMEZONE_DB_KEY

#define CREDENTIAL_BUFFER_SIZE 64

enum SetupSyncProps {
  SetupSyncWifiSsidProp = 0, // wifi ssid
  SetupSyncWifiPassProp,     // wifi pass
  SetupSyncIftttKeyProp,     // ifttt key (webhook)
  SetupSyncITimeKeyProp,     // time api key
  SetupSyncPropsDelimiter    // delimiter of the configuration states
};

/**
 * This actor holds the setup/credentials/sensitive information for the rest of the modules to use.
 */
// TODO credentials are to be stored in EEPROM
// TODO credentials are to be setup either via macros or via serial port
class SetupSync : public Actor {

private:
  const char *name;

  Buffer *ssid;
  Buffer *pass;
  Buffer *iftt;
  Buffer *time;
  Metadata *md;

public:
  SetupSync(const char *n) {
    name = n;

    ssid = new Buffer(CREDENTIAL_BUFFER_SIZE);
    pass = new Buffer(CREDENTIAL_BUFFER_SIZE);
    iftt = new Buffer(CREDENTIAL_BUFFER_SIZE);
    time = new Buffer(CREDENTIAL_BUFFER_SIZE);

    ssid->load(WIFI_SSID_STEADY);
    pass->load(WIFI_PASSWORD_STEADY);
    iftt->load(IFTTT_KEY);
    time->load(TIMEZONE_DB_KEY);
    md = new Metadata(n);
  }

  const char *getName() {
    return name;
  }

  void act() {}

  void getSetPropValue(int propIndex, GetSetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (SetupSyncWifiSsidProp):
        setPropValue(setMode, targetValue, NULL, ssid); // write only
        break;
      case (SetupSyncWifiPassProp):
        setPropValue(setMode, targetValue, NULL, pass); // write only
        break;
      case (SetupSyncIftttKeyProp):
        setPropValue(setMode, targetValue, NULL, iftt); // write only
        break;
      case (SetupSyncITimeKeyProp):
        setPropValue(setMode, targetValue, NULL, time); // write only
        break;
      default:
        break;
    }
    if (setMode != GetValue) {
      getMetadata()->changed();
    }
  }

  int getNroProps() {
    return SetupSyncPropsDelimiter;
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

  void getInfo(int infoIndex, Buffer *info) {}

  int getNroInfos() {
    return 0;
  }

  const char *getSsid() {
    return ssid->getBuffer();
  }

  void setSsid(const char *s) {
    ssid->load(s);
  }

  const char *getPass() {
    return pass->getBuffer();
  }

  void setPass(const char *s) {
    pass->load(s);
  }

  const char *getIfttt() {
    return iftt->getBuffer();
  }

  void setIfttt(const char *s) {
    iftt->load(s);
  }

  const char *getTimeKey() {
    return time->getBuffer();
  }

  void setTimeKey(const char *s) {
    time->load(s);
  }

  Metadata *getMetadata() {
    return md;
  }

  bool isInitialized() {
    return ssid->getBuffer()[0] != '?' && pass->getBuffer()[0] != '?' && iftt->getBuffer()[0] != '?' && time->getBuffer()[0] != '?';
  }
};

#endif // SETUPSYNC_INC
