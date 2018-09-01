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

#ifndef BOTINOBE_API_HOST_BASE
#error "Must define BOTINOBE_API_HOST_BASE"
#endif

#define BOTINOBE_API_URL_BASE BOTINOBE_API_HOST_BASE "/api/v1/devices/" DEVICE_NAME
#define BOTINOBE_API_URL_POST_CURRENT BOTINOBE_API_URL_BASE "/actors/%s/reports"
#define BOTINOBE_API_URL_GET_TARGET BOTINOBE_API_URL_BASE "/actors/%s/targets/summary?consume=true&status=C"
#define BOTINOBE_API_URL_RESTORE_CURRENT BOTINOBE_API_URL_BASE "/actors/%s/current/last?status=X"

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
      if (connected) {
        for (int i = 0; i < bot->getActors()->size(); i++) {
          updateProps(i);
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
    Actor *actor = bot->getActors()->get(actorIndex);

    if (actor->getNroProps() == 0) {
      return; // nothing to be syncd
    }

    ParamStream httpBodyResponse;
    const char *actorName = actor->getName();

    log(CLASS_PROPSYNC, Info, "Sync: %s", actor->getName());

    if (!actor->getMetadata()->isRestored()) {
    	// Restore from previous status
      log(CLASS_PROPSYNC, Debug, "RestTarg:%s", actorName);
      urlAuxBuffer.fill(BOTINOBE_API_URL_RESTORE_CURRENT, actorName);
      int errorCodeRes = httpGet(urlAuxBuffer.getBuffer(), &httpBodyResponse);
      if (errorCodeRes == HTTP_OK) {
        JsonObject &json = httpBodyResponse.parse();
        bot->setPropsJson(json, actorIndex);
        actor->getMetadata()->restored();
      } else {
        log(CLASS_PROPSYNC, Warn, "KO: %d", errorCodeRes);
      }
    } else {
    	// Regular run
      log(CLASS_PROPSYNC, Debug, "LoadTarg:%s", actorName);
      urlAuxBuffer.fill(BOTINOBE_API_URL_GET_TARGET, actorName);
      int errorCodeGet = httpGet(urlAuxBuffer.getBuffer(), &httpBodyResponse);
      if (errorCodeGet == HTTP_OK) {
        JsonObject &json = httpBodyResponse.parse();
        bot->setPropsJson(json, actorIndex);
      } else {
        log(CLASS_PROPSYNC, Warn, "KO: %d", errorCodeGet);
      }

      bot->getPropsJson(&jsonAuxBuffer, actorIndex);
      urlAuxBuffer.fill(BOTINOBE_API_URL_POST_CURRENT, actorName);
      log(CLASS_PROPSYNC, Debug, "UpdCurr:%s", actorName);
      httpPost(urlAuxBuffer.getBuffer(), jsonAuxBuffer.getBuffer(), NULL); // best effort to push current status
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
