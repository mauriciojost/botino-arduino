#ifndef MODULE_SETTINGS_INC
#define MODULE_SETTINGS_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>

#define STATUS_BUFFER_SIZE 64
#define TARGET_BUFFER_SIZE 32

#define CLASS_BOTINO_SETTINGS "SS"
#define SKIP_UPDATES_CODE "skip"
#define UPDATE_COMMAND "update %s"

enum BotinoSettingsProps {
  BotinoSettingsLcdLogsProp = 0,    // boolean, define if the device display logs in LCD
  BotinoSettingsStatusProp,         // string, defines the current general status of the device (vcc level, heap, etc)
  BotinoSettingsFsLogsProp,         // boolean, define if logs are to be dumped in the file system (only in debug mode)
  BotinoSettingsUpdateTargetProp,   // string, target version of firmware to update to
  BotinoSettingsUpdateFreqProp,     // string, frequency of updates of target
  BotinoSettingsWifiSsidBackupProp, // string, ssid for backup wifi network
  BotinoSettingsWifiPassBackupProp, // string, pass for backup wifi network
  BotinoSettingsPropsDelimiter
};

class BotinoSettings : public Actor {

private:
  const char *name;
  bool lcdLogs;
  Buffer *status;
  bool fsLogs;
  Buffer *target;
  Buffer *ssidb;
  Buffer *passb;
  Metadata *md;
  void (*command)(const char *);

public:
  BotinoSettings(const char *n) {
    name = n;
    lcdLogs = false;
    status = new Buffer(STATUS_BUFFER_SIZE);
    fsLogs = false;
    target = new Buffer(TARGET_BUFFER_SIZE);
    target->load(SKIP_UPDATES_CODE);
    ssidb = new Buffer(20);
    ssidb->load("defaultssid");
    passb = new Buffer(20);
    passb->load("defaultssid");
    md = new Metadata(n);
    md->getTiming()->setFreq("~24h");
    command = NULL;
  }

  void setup(void (*cmd)(const char *)) {
    command = cmd;
  }

  const char *getName() {
    return name;
  }

  int getNroProps() {
    return BotinoSettingsPropsDelimiter;
  }

  void act() {
    if (getTiming()->matches()) {
      const char *currVersion = STRINGIFY(PROJ_VERSION);
      if (!target->equals(currVersion) && !target->equals(SKIP_UPDATES_CODE)) {
        log(CLASS_BOTINO_SETTINGS, Warn, "Have to update '%s'->'%s'", currVersion, target->getBuffer());
        if (command != NULL) {
          Buffer aux(64);
          command(aux.fill(UPDATE_COMMAND, target->getBuffer()));
        }
      }
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (BotinoSettingsLcdLogsProp):
        return DEBUG_PROP_PREFIX "lcdlogs";
      case (BotinoSettingsStatusProp):
        return STATUS_PROP_PREFIX "status";
      case (BotinoSettingsFsLogsProp):
        return DEBUG_PROP_PREFIX "fslogs";
      case (BotinoSettingsUpdateTargetProp):
        return ADVANCED_PROP_PREFIX "target";
      case (BotinoSettingsUpdateFreqProp):
        return ADVANCED_PROP_PREFIX "freq";
      case (BotinoSettingsWifiSsidBackupProp):
        return SENSITIVE_PROP_PREFIX "ssidb";
      case (BotinoSettingsWifiPassBackupProp):
        return SENSITIVE_PROP_PREFIX "passb";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (BotinoSettingsLcdLogsProp):
        setPropBoolean(m, targetValue, actualValue, &lcdLogs);
        break;
      case (BotinoSettingsStatusProp):
        setPropValue(m, targetValue, actualValue, status);
        break;
      case (BotinoSettingsFsLogsProp):
        setPropBoolean(m, targetValue, actualValue, &fsLogs);
        break;
      case (BotinoSettingsUpdateTargetProp):
        setPropValue(m, targetValue, actualValue, target);
        break;
      case (BotinoSettingsUpdateFreqProp):
        setPropTiming(m, targetValue, actualValue, getTiming());
        break;
      case (BotinoSettingsWifiSsidBackupProp):
        setPropValue(m, targetValue, actualValue, ssidb);
        break;
      case (BotinoSettingsWifiPassBackupProp):
        setPropValue(m, targetValue, actualValue, passb);
        break;
      default:
        break;
    }
    if (m != GetValue) {
      getMetadata()->changed();
    }
  }

  Metadata *getMetadata() {
    return md;
  }

  Buffer *getStatus() {
    return status;
  }

  bool fsLogsEnabled() {
    return fsLogs;
  }

  bool getLcdLogs() {
    return lcdLogs;
  }

  Buffer *getBackupWifiSsid() {
    return ssidb;
  }

  Buffer *getBackupWifiPass() {
    return passb;
  }
};

#endif // MODULE_SETTINGS_INC
