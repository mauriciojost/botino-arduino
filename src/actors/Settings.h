#ifndef GLOBAL_INC
#define GLOBAL_INC

/**
 * Settings
 *
 * Holds global settings for the device (like log level, debug mode, etc.).
 *
 */

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Integer.h>

#define CLASS_SETTINGS "ST"


#define INFO_BUFFER_LENGTH 256

#ifndef WIFI_SSID_STEADY
#define WIFI_SSID_STEADY "???"
#endif // WIFI_SSID_STEADY

#ifndef WIFI_PASSWORD_STEADY
#define WIFI_PASSWORD_STEADY "???"
#endif // WIFI_PASSWORD_STEADY

#ifndef PERIOD_SEC
#define PERIOD_SEC 60
#endif // PERIOD_SEC

#define PERIOD_MSEC (PERIOD_SEC * 1000)


#define CREDENTIAL_BUFFER_SIZE 64


enum SettingsProps {
  SettingsDebugProp = 0,          // boolean, define if the device is in debug mode
  SettingsDeepSleepProp,          // boolean, define if the device is in deep sleep mode
  SettingsButtonRoutineLimitProp, // integer, define the first X routines that are randomly executed when the button is pressed
  SettingsPeriodMsProp,           // period in msec for the device to sleep
  SettingsWifiSsidProp,           // wifi ssid
  SettingsWifiPassProp,           // wifi pass
  SettingsPropsDelimiter          // amount of properties
};

class Settings : public Actor {

private:
  const char *name;
  bool devDebug;
  bool deepSleep;
  int periodms;
  int buttonRoutineUntil;
  Buffer *infoBuffer;
  Buffer *ssid;
  Buffer *pass;
  Metadata *md;

public:
  Settings(const char *n) {
    name = n;
    ssid = new Buffer(CREDENTIAL_BUFFER_SIZE);
    ssid->load(WIFI_SSID_STEADY);

    pass = new Buffer(CREDENTIAL_BUFFER_SIZE);
    pass->load(WIFI_PASSWORD_STEADY);

    infoBuffer = new Buffer(INFO_BUFFER_LENGTH);
    devDebug = true;
#ifdef DEEP_SLEEP_MODE_ENABLED
    deepSleep = true;
#else // DEEP_SLEEP_MODE_ENABLED
    deepSleep = false;
#endif // DEEP_SLEEP_MODE_ENABLED
    buttonRoutineUntil = 4;
    infoBuffer->clear();
    periodms = PERIOD_MSEC;
    md = new Metadata(n);
  }

  const char *getName() {
    return name;
  }

  void act() {}

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (SettingsWifiSsidProp):
        return "_wifissid"; // with obfuscation (starts with _)
      case (SettingsWifiPassProp):
        return "_wifipass"; // with obfuscation (starts with _)
      case (SettingsDebugProp):
        return "debug";
      case (SettingsDeepSleepProp):
        return "deepsleep";
      case (SettingsPeriodMsProp):
        return "periodms";
      case (SettingsButtonRoutineLimitProp):
        return "btnrout";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (SettingsDebugProp):
        setPropBoolean(m, targetValue, actualValue, &devDebug);
        break;
      case (SettingsDeepSleepProp):
#ifdef DEEP_SLEEP_MODE_ENABLED
        setPropBoolean(m, targetValue, actualValue, &deepSleep);
#else // DEEP_SLEEP_MODE_ENABLED
        deepSleep = false;
        if (actualValue != NULL) {
          Boolean b(false);
          actualValue->load(&b);
        }
#endif // DEEP_SLEEP_MODE_ENABLED
        break;
      case (SettingsPeriodMsProp):
        setPropInteger(m, targetValue, actualValue, &periodms);
        break;
      case (SettingsButtonRoutineLimitProp):
        setPropInteger(m, targetValue, actualValue, &buttonRoutineUntil);
        break;
      case (SettingsWifiSsidProp):
        setPropValue(m, targetValue, actualValue, ssid);
        break;
      case (SettingsWifiPassProp):
        setPropValue(m, targetValue, actualValue, pass);
        break;
      default:
        break;
    }
    if (m != GetValue) {
      getMetadata()->changed();
    }
  }

  int getNroProps() {
    return SettingsPropsDelimiter;
  }

  void getInfo(int infoIndex, Buffer *info) {
    info->load(infoBuffer);
  }

  int getNroInfos() {
    return 1;
  }

  Metadata *getMetadata() {
    return md;
  }

  int getNroRoutinesForButton() {
    return buttonRoutineUntil;
  }

  bool getDebug() {
    return devDebug;
  }

  void setDebug(bool b) {
    if (b != devDebug) {
      getMetadata()->changed();
    }
    devDebug = b;
  }

  void setInfo(const char *s) {
    infoBuffer->fill(s);
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

  bool inDeepSleepMode() {
  	return deepSleep;
  }

  int periodMsec() {
  	return periodms;
  }

};

#endif // GLOBAL_INC
