#ifndef MODULE_SETTINGS_INC
#define MODULE_SETTINGS_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>

#define STATUS_BUFFER_SIZE 64

#define CLASS_BOTINO_SETTINGS "SS"
#define DEFAULT_FS_LOGS_LENGTH 32

enum BotinoSettingsProps {
  BotinoSettingsLcdLogsProp = 0,  // boolean, define if the device display logs in LCD
  BotinoSettingsStatusProp,       // string, defines the current general status of the device (vcc level, heap, etc)
  BotinoSettingsFsLogsProp,       // boolean, define if logs are to be dumped in the file system (only in debug mode)
  BotinoSettingsFsLengthLogsProp, // integer, define the length of the line in the logs
  BotinoSettingsPropsDelimiter
};

class BotinoSettings : public Actor {

private:
  const char *name;
  bool lcdLogs;
  Buffer *status;
  bool fsLogs;
  int fsLogsLength;
  Metadata *md;

public:
  BotinoSettings(const char *n) {
    name = n;
    lcdLogs = true;
    status = new Buffer(STATUS_BUFFER_SIZE);
    fsLogs = true;
    fsLogsLength = DEFAULT_FS_LOGS_LENGTH;
    md = new Metadata(n);
    md->getTiming()->setFreq("~24h");
  }

  const char *getName() {
    return name;
  }

  int getNroProps() {
    return BotinoSettingsPropsDelimiter;
  }

  Act act(Metadata *md) {
    return Act("");
  }

  CmdExecStatus command(Cmd *) {
    return NotFound;
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (BotinoSettingsLcdLogsProp):
        return DEBUG_PROP_PREFIX "lcdlogs";
      case (BotinoSettingsStatusProp):
        return STATUS_PROP_PREFIX "status";
      case (BotinoSettingsFsLogsProp):
        return DEBUG_PROP_PREFIX "fslogs";
      case (BotinoSettingsFsLengthLogsProp):
        return DEBUG_PROP_PREFIX "fsll";
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
      case (BotinoSettingsFsLengthLogsProp):
        setPropInteger(m, targetValue, actualValue, &fsLogsLength);
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

  int getFsLogsLength() {
    return fsLogsLength;
  }

  bool getLcdLogs() {
    return lcdLogs;
  }
};

#endif // MODULE_SETTINGS_INC
