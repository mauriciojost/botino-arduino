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

#ifndef MAIN4INOSERVER_API_HOST_BASE
#error "Must define MAIN4INOSERVER_API_HOST_BASE"
#endif

#define MAIN4INOSERVER_API_URL_BASE MAIN4INOSERVER_API_HOST_BASE "/api/v1/devices/" DEVICE_NAME
#define MAIN4INOSERVER_API_URL_POST_CURRENT MAIN4INOSERVER_API_URL_BASE "/actors/%s/reports"
#define MAIN4INOSERVER_API_URL_GET_TARGET MAIN4INOSERVER_API_URL_BASE "/actors/%s/targets/summary?consume=true&status=C"
#define MAIN4INOSERVER_API_URL_RESTORE_CURRENT MAIN4INOSERVER_API_URL_BASE "/actors/%s/reports/last"
#define MAIN4INOSERVER_API_URL_GET_PROPS_TO_CONSUME_COUNT MAIN4INOSERVER_API_URL_BASE "/targets/count?status=C"

#define SENSITIVE_PROP_PREFIX '_'

enum PropSyncProps {
  PropSyncFreqProp = 0,
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
  Metadata* md;
  Buffer<128> urlAuxBuffer;
  Buffer<MAX_JSON_STR_LENGTH> jsonAuxBuffer;
  bool (*initWifiFunc)();
  int (*httpGet)(const char *url, ParamStream *response);
  int (*httpPost)(const char *url, const char *body, ParamStream *response);
  bool boot;

public:
  PropSync(const char *n) {
    name = n;
    bot = NULL;
    initWifiFunc = NULL;
    httpGet = NULL;
    httpPost = NULL;
    md = new Metadata(n);
    md->getTiming()->setFrek(201010560);
    boot = true;
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
    if (getTiming()->matches()) {
      log(CLASS_PROPSYNC, Info, "Propsync starts");
      bool connected = initWifiFunc();
      int toConsumeProps = getToConsumeProps();
      if (connected) {
        for (int i = 0; i < bot->getActors()->size(); i++) {
        	handleActor(i, toConsumeProps);
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

  // Restore properties from previous status
  void restoreActor(int actorIndex) {
    Actor *actor = bot->getActors()->get(actorIndex);
    const char *actorName = actor->getName();
    ParamStream httpBodyResponse(&jsonAuxBuffer);

    log(CLASS_PROPSYNC, Debug, "RestTarg:%s", actorName);
    urlAuxBuffer.fill(MAIN4INOSERVER_API_URL_RESTORE_CURRENT, actorName);
    int errorCodeRes = httpGet(urlAuxBuffer.getBuffer(), &httpBodyResponse);
    if (errorCodeRes == HTTP_OK) { // data stored in the server and retrieved
      log(CLASS_PROPSYNC, Debug, "OK: %d", errorCodeRes);
      JsonObject &json = httpBodyResponse.parse();
      bot->setPropsJson(json, actorIndex);
      actor->getMetadata()->restored();
    } else if (errorCodeRes == HTTP_NO_CONTENT) { // no data stored in the server
      log(CLASS_PROPSYNC, Debug, "OK: %d", errorCodeRes);
      actor->getMetadata()->restored();
    } else { // failure, will retry after
      log(CLASS_PROPSYNC, Warn, "KO: %d", errorCodeRes);
    }
  }


  void syncActor(int toConsumeProps, int actorIndex) {
    Actor *actor = bot->getActors()->get(actorIndex);
    const char *actorName = actor->getName();

    // Regular run
    if (toConsumeProps == 0) {
      log(CLASS_PROPSYNC, Debug, "Skip LoadTarg:%s(no remote changes)", actorName);
    } else {
      ParamStream httpBodyResponse(&jsonAuxBuffer);
      log(CLASS_PROPSYNC, Debug, "LoadTarg:%s", actorName);
      urlAuxBuffer.fill(MAIN4INOSERVER_API_URL_GET_TARGET, actorName);
      int errorCodeGet = httpGet(urlAuxBuffer.getBuffer(), &httpBodyResponse);
      if (errorCodeGet == HTTP_OK) {
        log(CLASS_PROPSYNC, Debug, "OK: %d", errorCodeGet);
        JsonObject &json = httpBodyResponse.parse();
        bot->setPropsJson(json, actorIndex);
      } else {
        log(CLASS_PROPSYNC, Warn, "KO: %d", errorCodeGet);
      }
    }

    if (actor->getMetadata()->hasChanged()) {
      bot->getPropsJson(&jsonAuxBuffer, actorIndex, SENSITIVE_PROP_PREFIX);
      urlAuxBuffer.fill(MAIN4INOSERVER_API_URL_POST_CURRENT, actorName);
      log(CLASS_PROPSYNC, Debug, "UpdCurr:%s", actorName);
      int errorCodePo = httpPost(urlAuxBuffer.getBuffer(), jsonAuxBuffer.getBuffer(), NULL);
      if (errorCodePo == HTTP_CREATED) {
        log(CLASS_PROPSYNC, Debug, "OK: %d", errorCodePo);
        actor->getMetadata()->clearChanged();
      } else {
        log(CLASS_PROPSYNC, Warn, "KO: %d", errorCodePo);
      }
    } else {
      log(CLASS_PROPSYNC, Debug, "Skip UpdCurr:%s(no local changes)", actorName);
    }
  }

  void handleActor(int actorIndex, int toConsumeProps) {

    Actor *actor = bot->getActors()->get(actorIndex);
    if (actor->getNroProps() == 0) {
      return; // nothing to be syncd
    }

    log(CLASS_PROPSYNC, Info, "Sync: %s", actor->getName());
    if (!actor->getMetadata()->isRestored()) { // actor data not restored yet
    	restoreActor(actorIndex);
    } else {
    	syncActor(toConsumeProps, actorIndex);
    }
  }

  int getToConsumeProps() {
    ParamStream httpBodyResponse(&jsonAuxBuffer);
    log(CLASS_PROPSYNC, Info, "Props to consume?");
    urlAuxBuffer.fill(MAIN4INOSERVER_API_URL_GET_PROPS_TO_CONSUME_COUNT);
    int errorCodeGet = httpGet(urlAuxBuffer.getBuffer(), &httpBodyResponse);
    if (errorCodeGet == HTTP_OK) {
      JsonObject &json = httpBodyResponse.parse();
      if (json.success() && json.containsKey("count")) {
        const char *cnt = json["count"].as<char *>();
        if (cnt != NULL) {
          int v = atoi(cnt);
          log(CLASS_PROPSYNC, Info, "Count: %d", v);
        	return v;
        }
      }
      log(CLASS_PROPSYNC, Info, "Can't determine count");
      return -1;
    } else {
      log(CLASS_PROPSYNC, Warn, "KO: %d", errorCodeGet);
      return -1;
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (PropSyncFreqProp):
        return "freq";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (PropSyncFreqProp): {
        long freq = md->getTiming()->getFrek();
        setPropLong(m, targetValue, actualValue, &freq);
        if (m == SetCustomValue) {
          md->getTiming()->setFrek(freq);
        }
      } break;
      default:
        break;
    }
    if (m != GetValue) {
    	getMetadata()->changed();
    }
  }

  int getNroProps() {
    return PropSyncPropsgDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Metadata *getMetadata() {
    return md;
  }

};

#endif // PROPSYNC_INC
