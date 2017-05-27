#include <actors/Messenger.h>
#include <actors/ParamStream.h>
#include <main4ino/Clock.h>
#include <main4ino/Boolean.h>

#define CLASS "Messenger"

#define DELAY_UNIT_MS 5000
#define MAX_URL_EFF_LENGTH 100
#define MAX_WIFI_CONNECTION_ATTEMPTS 100

#define DWEET_IO_API_URL_BASE "http://dweet.io"
#define DWEET_IO_API_URL_BASE_POST DWEET_IO_API_URL_BASE "/dweet/for/%s" // device
#define DWEET_IO_API_URL_BASE_GET DWEET_IO_API_URL_BASE "/get/latest/dweet/for/%s-target" // device
#define TIMEZONE_DB_API_URL_BASE_GET "http://api.timezonedb.com/v2/get-time-zone?key=%s&format=json&by=zone&zone=%s"

#ifndef WIFI_SSID
#error "Must provide WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#error "Must provide WIFI_PASSWORD"
#endif
#ifndef DWEET_IO_API_TOKEN
#error "Must provide DWEET_IO_API_TOKEN"
#endif
#ifndef TIMEZONE_DB_KEY
#error "Must provide TIMEZONE_DB_KEY"
#endif
#ifndef TIMEZONE_DB_ZONE
#error "Must provide TIMEZONE_DB_ZONE"
#endif

Messenger::Messenger(const char* n): freqConf(OnceEvery5Minutes) {
  name = n;
  bot = NULL;
}

void Messenger::setBot(WebBot* b) {
  bot = b;
}

const char *Messenger::getName() {
  return name;
}

void Messenger::connectToWifi() {
#ifndef UNIT_TEST

  static bool configured = false;
  int attempts = 0;
  log(CLASS, Info, "Status: ", (int)WiFi.status());
  while(WiFi.status() != WL_CONNECTED || !configured) {
    log(CLASS, Info, "Connecting...");
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    configured = true;
    while (WiFi.status() != WL_CONNECTED) {
      delay(DELAY_UNIT_MS);
      log(CLASS, Info, ".");
      if (attempts++ > MAX_WIFI_CONNECTION_ATTEMPTS) {
        attempts = 0;
        break;
      }
    }
    delay(DELAY_UNIT_MS);
  }
#endif // UNIT_TEST
}

void Messenger::cycle(bool cronMatches) {
  if (bot == NULL) {
    return;
  }
  if (cronMatches) {
    connectToWifi();
    updateBotProperties();
    updateClockProperties();
  }
}

void Messenger::updateClockProperties() {
  ParamStream s;
#ifndef UNIT_TEST
  int errorCode;
  Buffer<MAX_URL_EFF_LENGTH> urlAux;

  HTTPClient httpGet;
  url.clear();
  url.fill(TIMEZONE_DB_API_URL_BASE_GET, TIMEZONE_DB_KEY, TIMEZONE_DB_ZONE);
  httpGet.begin(url.getBuffer());
  log(CLASS, Info, "Client connected to: ", url.getBuffer());
  errorCode = httpGet.GET();
  log(CLASS, Info, "Response code to GET: ", errorCode);
  httpGet.writeToStream(&s);
  httpGet.end();

  JsonObject& json = s.parse();

  if (json.containsKey("formatted")) {
    const char* formatted = json["formatted"].as<char *>();
    Buffer<8> time(formatted + 11);
    Boolean autoAdjust(true);
    bot->getClock()->setProp(ClockConfigStateAutoAdjustFactor, SetValue, &autoAdjust, NULL);
    bot->getClock()->setProp(ClockConfigStateHhMmSs, SetValue, &time, NULL);
  } else {
    log(CLASS, Warn, "Failed to parse 'formatted'");
  }
#endif // UNIT_TEST
  s.flush();
}

void Messenger::updateBotProperties() {
  ParamStream s;
#ifndef UNIT_TEST
  int errorCode;
  Buffer<MAX_URL_EFF_LENGTH> urlAux;

  HTTPClient httpPost;

  bot->getPropsUrl(&url);
  url.prepend("?");
  urlAux.fill(DWEET_IO_API_URL_BASE_POST, DEVICE_NAME);
  url.prepend(urlAux.getBuffer());

  httpPost.begin(url.getBuffer());
  httpPost.addHeader("Content-Type", "application/json");
  httpPost.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);
  log(CLASS, Info, "Client connected to: ", url.getBuffer());
  errorCode = httpPost.POST("");
  log(CLASS, Info, "Response code to POST: ", errorCode);
  httpPost.writeToStream(&Serial);
  httpPost.end();

  HTTPClient httpGet;
  url.clear();
  url.fill(DWEET_IO_API_URL_BASE_GET, DEVICE_NAME);
  httpGet.begin(url.getBuffer());
  httpGet.addHeader("Content-Type", "application/json");
  httpGet.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);
  log(CLASS, Info, "Client connected to: ", url.getBuffer());
  errorCode = httpGet.GET();
  log(CLASS, Info, "Response code to GET: ", errorCode);
  httpGet.writeToStream(&s);
  httpGet.end();

  JsonObject& json = s.parse();

  if (json.containsKey("with")) {
    JsonObject& withJson = json["with"][0];
    if (withJson.containsKey("content")) {
      JsonObject& content = withJson["content"];
      bot->setPropsJsonFlat(content);
    } else {
      log(CLASS, Warn, "Failed to parse 'content'");
    }
  } else {
    log(CLASS, Warn, "Failed to parse 'with'");
  }
#endif // UNIT_TEST
  s.flush();
}


void Messenger::getActuatorValue(Value* value) { }

void Messenger::setProp(int propIndex, SetMode set, const Value* targetValue, Value* actualValue) { }

int Messenger::getNroProps() { return 0; }

const char* Messenger::getPropName(int propIndex) {
  switch (propIndex) {
  default:
    return "";
  }
}

void Messenger::getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH>* info) { }

int Messenger::getNroInfos() { return 0; }

FreqConf *Messenger::getFrequencyConfiguration() { return &freqConf; }
