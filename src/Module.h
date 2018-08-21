#ifndef MODULE_INC
#define MODULE_INC

#include "actors/Messages.h"
#include "actors/Quotes.h"
#include "actors/Settings.h"
#include <Pinout.h>
#include <actors/Body.h>
#include <actors/ClockSync.h>
#include <actors/Images.h>
#include <actors/PropSync.h>
#include <actors/SetupSync.h>
#include <actors/Ifttt.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Array.h>
#include <main4ino/Clock.h>
#include <main4ino/SerBot.h>

#define CLASS_MODULE "MD"

/**
 * This class represents the integration of all components (LCD, buttons, buzzer, etc).
 */
class Module {

private:
  SetupSync *setupSync;
  PropSync *propSync;
  ClockSync *clockSync;
  Array<Actor *> *actors;
  Clock *clock;
  Settings *settings;
  SerBot *bot;
  Body *body;
  Quotes *quotes;
  Images *images;
  Messages *messages;
  Ifttt *ifttt;

public:
  Module() {

    setupSync = new SetupSync("setupsync");
    propSync = new PropSync("propsync");
    clockSync = new ClockSync("clocksync");
    clock = new Clock("clock");
    settings = new Settings("settings");
    quotes = new Quotes("quotes");
    body = new Body("body");
    images = new Images("images");
    messages = new Messages("messages");
    ifttt = new Ifttt("ifttt");

    actors = new Array<Actor *>(10);
    actors->set(0, (Actor *)setupSync);
    actors->set(1, (Actor *)propSync);
    actors->set(2, (Actor *)clockSync);
    actors->set(3, (Actor *)clock);
    actors->set(4, (Actor *)settings);
    actors->set(5, (Actor *)quotes);
    actors->set(6, (Actor *)body);
    actors->set(7, (Actor *)images);
    actors->set(8, (Actor *)messages);
    actors->set(9, (Actor *)ifttt);

    bot = new SerBot(clock, actors);

    propSync->setBot(bot);
    clockSync->setClock(bot->getClock());
    body->setQuotes(quotes);
    body->setImages(images);
    body->setMessages(messages);

    bot->setMode(RunMode);
  }

  void setup(
    void (*lcdImg)(char img, uint8_t bitmap[]),
    void (*arms)(int left, int right, int steps),
    void (*messageFunc)(int line, const char *str, int size),
    void (*ios)(char led, bool v),
    bool (*initWifiInit)(),
    bool (*initWifiSteady)(),
    int (*httpPost)(const char *url, const char *body, ParamStream *response),
    int (*httpGet)(const char *url, ParamStream *response)
  ) {

    body->setLcdImgFunc(lcdImg);
    body->setArmsFunc(arms);
    body->setMessageFunc(messageFunc);
    body->setIosFunc(ios);
    propSync->setInitWifi(initWifiSteady);
    propSync->setHttpPost(httpPost);
    propSync->setHttpGet(httpGet);
    clockSync->setInitWifi(initWifiSteady);
    clockSync->setHttpGet(httpGet);
    setupSync->setInitWifiSteady(initWifiSteady);
    setupSync->setInitWifiInit(initWifiInit);
    setupSync->setHttpGet(httpGet);
    quotes->setHttpGet(httpGet);
    quotes->setInitWifi(initWifiSteady);
    ifttt->setInitWifi(initWifiSteady);
    ifttt->setHttpPost(httpPost);
  }

  void loop(bool mode, bool set, bool cycle) {

    TimingInterrupt interruptType = TimingInterruptNone;

    if (cycle) {
      interruptType = TimingInterruptCycle;
    }

    // execute a cycle on the bot
    bot->cycle(mode, set, interruptType);
  }

  // All getters should be removed, and initialization of these instances below should
  // be done in Module itself. This should help decrease the size of
  Bot *getBot() {
    return bot;
  }

  Clock *getClock() {
    return clock;
  }

  Settings *getSettings() {
    return settings;
  }

  Body *getBody() {
    return body;
  }

  SetupSync *getSetupSync() {
    return setupSync;
  }

  PropSync *getPropSync() {
    return propSync;
  }

  ClockSync *getClockSync() {
    return clockSync;
  }

  Quotes *getQuotes() {
    return quotes;
  }

  Ifttt *getIfttt() {
    return ifttt;
  }

};

#endif // MODULE_INC
