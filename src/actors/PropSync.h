#ifndef PROPSYNC_INC
#define PROPSYNC_INC

#include <HttpCodes.h>
#include <actors/ParamStream.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Bot.h>
#include <main4ino/Clock.h>
#include <main4ino/Misc.h>
#include <main4ino/SerBot.h>

#define CLASS_PROPSYNC "PY"

#define DWEET_IO_API_URL_POST "http://dweet.io/dweet/for/" DEVICE_NAME "-%s-current"
#define DWEET_IO_API_URL_GET "http://dweet.io/get/latest/dweet/for/" DEVICE_NAME "-%s-target"
#define DWEET_IO_API_URL_REPORT "http://dweet.io/get/latest/dweet/for/" DEVICE_NAME "-%s-report"

enum PropSyncProps {
  PropSyncFreqProp = 0,
  PropSyncUpdatePropsProp,
  PropSyncUpdateInfosProp,
  PropSyncPropsgDelimiter // count of properties
};

/**
 * This actor exchanges status via HTTP to synchronize
 * properties with target property values provided by an user
 * on a centralized server, polled regularly.
 */
class PropSync : public Actor {

private:
  const char *name;
  SerBot *bot;
  Timing freqConf; // configuration of the frequency at which this actor will get triggered
  Buffer<128> urlAuxBuffer;
  Buffer<MAX_JSON_STR_LENGTH> jsonAuxBuffer;
  bool (*initWifiFunc)();
  int (*httpGet)(const char *url, ParamStream *response);
  int (*httpPost)(const char *url, const char *body, ParamStream *response);
  bool updatePropsEnabled;
  bool updateReportEnabled;

public:
  PropSync(const char *n) {
    name = n;
    bot = NULL;
    initWifiFunc = NULL;
    httpGet = NULL;
    httpPost = NULL;
    freqConf.setFrequency(OnceEvery1Minute);
    updatePropsEnabled = true;
    updateReportEnabled = false;
  }

  void setBot(SerBot *b) {
    bot = b;
  }

  const char *getName() {
    return name;
  }

  void act() {
    if (bot == NULL || initWifiFunc == NULL || httpGet == NULL || httpPost == NULL) {
      log(CLASS_PROPSYNC, Error, "Init needed");
      return;
    }
    if (freqConf.matches()) {
      log(CLASS_PROPSYNC, Debug, "Propsync starts");
      bool connected = initWifiFunc();
      if (connected) {
        for (int i = 0; i < bot->getActors()->size(); i++) {
          if (updatePropsEnabled) {
            updateProps(i);
          }
          if (updateReportEnabled) {
            updateReport(i);
          }
        }
      }
    }
  }

  void setInitWifi(bool (*f)()) {
    initWifiFunc = f;
  }

  void setHttpGet(int (*h)(const char *url, ParamStream *response)) {
    httpGet = h;
  }

  void setHttpPost(int (*h)(const char *url, const char *body, ParamStream *response)) {
    httpPost = h;
  }

  void updateProps(int actorIndex) {
    ParamStream httpBodyResponse;
    Actor *actor = bot->getActors()->get(actorIndex);

    urlAuxBuffer.fill(DWEET_IO_API_URL_GET, actor->getName());
    int errorCode = httpGet(urlAuxBuffer.getBuffer(), &httpBodyResponse);
    if (errorCode == HTTP_OK) {
      JsonObject &json = httpBodyResponse.parse();
      if (json.containsKey("with")) {
        JsonObject &withJson = json["with"][0];
        if (withJson.containsKey("content")) {
          JsonObject &content = withJson["content"];
          log(CLASS_PROPSYNC, Debug, "SetProp:%s", actor->getName());
          bot->setPropsJson(content, actorIndex);
        } else {
          log(CLASS_PROPSYNC, Info, "No 'content'");
        }
      } else {
        log(CLASS_PROPSYNC, Info, "Inv. JSON(no 'with')");
      }
    } else {
      log(CLASS_PROPSYNC, Warn, "KO: %d", errorCode);
    }

    bot->getPropsJson(&jsonAuxBuffer, actorIndex);
    urlAuxBuffer.fill(DWEET_IO_API_URL_POST, actor->getName());
    log(CLASS_PROPSYNC, Debug, "DumpProp:%s", actor->getName());
    httpPost(urlAuxBuffer.getBuffer(), jsonAuxBuffer.getBuffer(), NULL); // best effort
  }

  void updateReport(int actorIndex) {
    ParamStream httpBodyResponse;
    Actor *actor = bot->getActors()->get(actorIndex);
    Buffer<MAX_JSON_STR_LENGTH> jsonAuxBuffer;
    bot->getInfosJson(&jsonAuxBuffer, actorIndex);
    urlAuxBuffer.fill(DWEET_IO_API_URL_REPORT, actor->getName());
    log(CLASS_PROPSYNC, Debug, "DumpRepo:%s", actor->getName());
    httpPost(urlAuxBuffer.getBuffer(), jsonAuxBuffer.getBuffer(), NULL); // best effort
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (PropSyncFreqProp):
        return "freq";
      case (PropSyncUpdatePropsProp):
        return "updateprops";
      case (PropSyncUpdateInfosProp):
        return "updateinfos";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (PropSyncFreqProp): {
        long freq = freqConf.getCustom();
        setPropLong(m, targetValue, actualValue, &freq);
        if (m == SetCustomValue) {
          freqConf.setCustom(freq);
          freqConf.setFrequency(Custom);
        }
      } break;
      case (PropSyncUpdatePropsProp): {
        setPropBoolean(m, targetValue, actualValue, &updatePropsEnabled);
      } break;
      case (PropSyncUpdateInfosProp): {
        setPropBoolean(m, targetValue, actualValue, &updateReportEnabled);
      } break;
      default:
        break;
    }
  }

  int getNroProps() {
    return PropSyncPropsgDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &freqConf;
  }
};

#endif // PROPSYNC_INC
