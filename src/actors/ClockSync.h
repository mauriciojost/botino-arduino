#ifndef CLOCKSYNC_INC
#define CLOCKSYNC_INC

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
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  bool (*initWifiFunc)();
  int (*httpGet)(const char *url, ParamStream *response);

public:
  ClockSync(const char *n) : freqConf(OnceEvery5Minutes) {
    name = n;
    clock = NULL;
    initWifiFunc = NULL;
    httpGet = NULL;
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
    if (freqConf.matches()) {
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
    if (errorCode > 0) {
      JsonObject &json = s.parse();
      if (json.containsKey("formatted")) {
        int y, mo, d, h, m, s;
        const char *formatted = json["formatted"].as<char *>(); // example: 2018-04-26 21:32:30
        log(CLASS_CLOCKSYNC, Debug, "Retrieved: %s", formatted);
        sscanf(formatted, "%04d-%02d-%02d %02d:%02d:%02d", &y, &mo, &d, &h, &m, &s);
        Buffer<8> time(formatted + 11);
        clock->setAutoAdjust(true);
        clock->set(DONT_CHANGE, h, m, s);
        clock->setAutoAdjust(false);
        clock->set(DONT_CHANGE, h, m, s);
        log(CLASS_CLOCKSYNC, Info, "Set time: %s", time.getBuffer());
      } else {
        log(CLASS_CLOCKSYNC, Warn, "Inv. JSON(no 'formatted')");
      }
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
    return &freqConf;
  }
};

#endif // CLOCKSYNC_INC
