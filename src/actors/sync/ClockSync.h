#ifndef CLOCKSYNC_INC
#define CLOCKSYNC_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/WebBot.h>
#include <main4ino/Misc.h>
#ifndef UNIT_TEST
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif // UNIT_TEST
#include <actors/sync/ParamStream.h>
#include <main4ino/Clock.h>
#include <main4ino/Boolean.h>

#define CLASS_CLOCKSYNC "CS"

#ifndef TIMEZONE_DB_KEY
#error "Must provide TIMEZONE_DB_KEY"
#endif
#ifndef TIMEZONE_DB_ZONE
#error "Must provide TIMEZONE_DB_ZONE"
#endif

#define TIMEZONE_DB_API_URL_GET "http://api.timezonedb.com/v2/get-time-zone?key=" TIMEZONE_DB_KEY "&format=json&by=zone&zone=" TIMEZONE_DB_ZONE

/**
* This actor exchanges status via HTTP to synchronize
* internal clock with time provided by the Internet.
*/
class ClockSync : public Actor {

private:
  const char *name;
  Clock *clock;
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  wl_status_t (*initWifiFunc)();

public:
  ClockSync(const char *n) : freqConf(OnceEvery1Minute) {
    name = n;
    clock = NULL;
    initWifiFunc = NULL;
  }

  void setClock(Clock *c) {
    clock = c;
  }

  const char *getName() {
    return name;
  }

  void act() {
    if (clock == NULL || initWifiFunc == NULL) {
      log(CLASS_CLOCKSYNC, Error, "Init needed");
      return;
    }
    if (freqConf.matches()) {
    	wl_status_t wifiStatus = initWifiFunc();
    	if (wifiStatus == WL_CONNECTED) {
        updateClockProperties();
    	}
    }
  }
  void setInitWifi(wl_status_t (*f)()) {
    initWifiFunc = f;
  }

  void updateClockProperties() {
    ParamStream s;
    int errorCode;

    HTTPClient client;
    client.begin(TIMEZONE_DB_API_URL_GET);
    errorCode = client.GET();
    log(CLASS_CLOCKSYNC, Info, "HTTP GET %s: %d", TIMEZONE_DB_API_URL_GET, errorCode);
    if (errorCode > 0) {
      client.writeToStream(&s);
      client.end();

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
        log(CLASS_CLOCKSYNC, Warn, "No 'formatted'");
      }
      s.flush();
    } else {
      log(CLASS_CLOCKSYNC, Error, "! %s", client.errorToString(errorCode).c_str());
    }
  }

  void setProp(int propIndex, SetMode set, const Value *targetValue, Value *actualValue) {}

  int getNroProps() { return 0; }

  const char *getPropName(int propIndex) { return ""; }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() { return 0; }

  Timing *getFrequencyConfiguration() { return &freqConf; }
};

#endif // CLOCKSYNC_INC
