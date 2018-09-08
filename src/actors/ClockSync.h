#ifndef CLOCKSYNC_INC
#define CLOCKSYNC_INC

#include <HttpCodes.h>
#include <actors/ParamStream.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Clock.h>
#include <main4ino/Misc.h>

#define CLASS_CLOCKSYNC "CS"

#ifndef TIMEZONE_DB_KEY
#define TIMEZONE_DB_KEY "xxx"
#endif
#ifndef TIMEZONE_DB_ZONE
#define TIMEZONE_DB_ZONE "xxx"
#endif

#define TIMEZONE_DB_API_URL_GET "http://api.timezonedb.com/v2/get-time-zone?key=%s&format=json&by=zone&zone=%s"

enum ClockSyncProps {
  ClockSyncZone0Prop = 0, // zone to take the time from (as per api.timezonedb.com/v2)
  ClockSyncPropsDelimiter // delimiter of the configuration states
};

/**
 * This actor exchanges status via HTTP to synchronize
 * internal clock with time provided by the Internet.
 */
class ClockSync : public Actor {

private:
  const char *name;
  Clock *clock;
  Metadata* md;
  Buffer<32> dbZone;
  Buffer<32> dbKey;
  bool (*initWifiFunc)();
  int (*httpGet)(const char *url, ParamStream *response);

public:
  ClockSync(const char *n) {
    name = n;
    clock = NULL;
    initWifiFunc = NULL;
    httpGet = NULL;
    md = new Metadata(n);
    md->getTiming()->setFrek(201126060);
    dbKey.fill(TIMEZONE_DB_KEY);
    dbZone.fill(TIMEZONE_DB_ZONE);
  }

  void setClock(Clock *c) {
    clock = c;
  }

  const char *getName() {
    return name;
  }

  void act() {
    if (clock == NULL || initWifiFunc == NULL || httpGet == NULL) {
      log(CLASS_CLOCKSYNC, Error, "Init needed");
      return;
    }
    if (getTiming()->matches()) {
      bool connected = initWifiFunc();
      if (connected) {
        updateClockProperties();
      }
    }
  }

  void setInitWifi(bool (*f)()) {
    initWifiFunc = f;
  }

  void setHttpGet(int (*h)(const char *url, ParamStream *response)) {
    httpGet = h;
  }

  void updateClockProperties() {
    log(CLASS_CLOCKSYNC, Info, "Updating clock");
    ParamStream s;
    Buffer<128> urlAuxBuffer;
    urlAuxBuffer.fill(TIMEZONE_DB_API_URL_GET, dbKey.getBuffer(), dbZone.getBuffer());
    int errorCode = httpGet(urlAuxBuffer.getBuffer(), &s);
    if (errorCode == HTTP_OK) {
      JsonObject &json = s.parse();
      if (json.containsKey("formatted")) {
        const char *formatted = json["formatted"].as<char *>(); // example: 2018-04-26 21:32:30
        int y, mo, d, h, m, s;
        log(CLASS_CLOCKSYNC, Debug, "Retrieved: '%s'", formatted);
        int parsed = sscanf(formatted, "%04d-%02d-%02d %02d:%02d:%02d", &y, &mo, &d, &h, &m, &s);
        if (parsed > 0) {
          clock->set(h, m, s, d, mo, y);
        } else {
          log(CLASS_CLOCKSYNC, Info, "Invalid time");
        }
      } else {
        log(CLASS_CLOCKSYNC, Warn, "Inv. JSON(no 'formatted')");
      }
    } else {
      log(CLASS_CLOCKSYNC, Warn, "KO: %d", errorCode);
    }
  }

  void getSetPropValue(int propIndex, GetSetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (ClockSyncZone0Prop):
        setPropValue(setMode, targetValue, actualValue, &dbZone);
        break;
      default:
        break;
    }
    if (setMode != GetValue) {
    	getMetadata()->changed();
    }
  }

  int getNroProps() {
    return 0;
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (ClockSyncZone0Prop):
        return "zone";
      default:
        return "";
    }
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  void setDbKey(const char *k) {
    dbKey.fill(k);
  }

  const char *getDbKey() {
    return dbKey.getBuffer();
  }

  int getNroInfos() {
    return 0;
  }

  Metadata *getMetadata() {
    return md;
  }

};

#endif // CLOCKSYNC_INC
