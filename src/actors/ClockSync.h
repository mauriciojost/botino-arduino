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
#error "Must provide TIMEZONE_DB_KEY"
#endif
#ifndef TIMEZONE_DB_ZONE
#error "Must provide TIMEZONE_DB_ZONE"
#endif

#define TIMEZONE_DB_API_URL_GET                                                                                                            \
  "http://api.timezonedb.com/v2/get-time-zone?key=" TIMEZONE_DB_KEY "&format=json&by=zone&zone=" TIMEZONE_DB_ZONE

/**
 * This actor exchanges status via HTTP to synchronize
 * internal clock with time provided by the Internet.
 */
class ClockSync : public Actor {

private:
  const char *name;
  Clock *clock;
  Timing timing; // configuration of the frequency at which this actor will get triggered
  bool (*initWifiFunc)();
  int (*httpGet)(const char *url, ParamStream *response);

public:
  ClockSync(const char *n) {
    name = n;
    clock = NULL;
    initWifiFunc = NULL;
    httpGet = NULL;
    timing.setFrequency(OnceEvery5Minutes);
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
    if (timing.matches()) {
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
    ParamStream s;
    int errorCode = httpGet(TIMEZONE_DB_API_URL_GET, &s);
    if (errorCode == HTTP_OK) {
      JsonObject &json = s.parse();
      if (json.containsKey("formatted")) {
        const char *formatted = json["formatted"].as<char *>(); // example: 2018-04-26 21:32:30
        int y, mo, d, h, m, s;
        log(CLASS_CLOCKSYNC, Debug, "Retrieved: '%s'", formatted);
        int parsed = sscanf(formatted, "%04d-%02d-%02d %02d:%02d:%02d", &y, &mo, &d, &h, &m, &s);
        if (parsed > 0) {
          Buffer<8> time(formatted + 11);
          clock->setAutoAdjust(true);
          clock->set(DONT_CHANGE, h, m, s);
          clock->setAutoAdjust(false);
          // TODO clock should support year, month and day too because this is too fragile
          // for instance: what happens if sync takes place at midnight?
          clock->set(DONT_CHANGE, h, m, s);
          log(CLASS_CLOCKSYNC, Info, "Set time: %s", time.getBuffer());
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

  void getSetPropValue(int propIndex, GetSetMode set, const Value *targetValue, Value *actualValue) {}

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

  Timing *getFrequencyConfiguration() {
    return &timing;
  }
};

#endif // CLOCKSYNC_INC
