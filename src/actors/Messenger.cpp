#include <actors/Messenger.h>

#define CLASS "Messenger"

#define DELAY_UNIT_MS 5000

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

#ifndef WIFI_SSID
#error "Must provide WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#error "Must provide WIFI_PASSWORD"
#endif
#ifndef API_TOKEN
#error "Must provide API_TOKEN"
#endif

#define DEVICE_ID "dev0"
#define API_URL_BASE "http://dweet.io"
#define API_URL_BASE_POST API_URL_BASE "/dweet/for/%s" // device
#define API_URL_BASE_GET API_URL_BASE "/get/latest/dweet/for/%s" // device

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
      if (attempts++ > 100) {
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

  connectToWifi();

#ifndef UNIT_TEST

  int errorCode;
  Buffer<MAX_JSON_STR_LENGTH> url;
  Buffer<100> urlAux;

  HTTPClient httpPost;

  bot->getPropsUrl(&url);
  url.prepend("?");
  urlAux.fill(API_URL_BASE_POST, DEVICE_ID);
  url.prepend(urlAux.getBuffer());

  httpPost.begin(url.getBuffer());
  httpPost.addHeader("Content-Type", "application/json");
  httpPost.addHeader("X-Auth-Token", API_TOKEN);
  log(CLASS, Info, "Client connected to: ", url.getBuffer());
  errorCode = httpPost.POST("");
  log(CLASS, Info, "Response code to POST: ", errorCode);
  httpPost.writeToStream(&Serial);
  httpPost.end();

  HTTPClient httpGet;
  url.clear();
  url.fill(API_URL_BASE_GET, DEVICE_ID);
  httpGet.begin(url.getBuffer());
  httpGet.addHeader("Content-Type", "application/json");
  httpGet.addHeader("X-Auth-Token", API_TOKEN);
  log(CLASS, Info, "Client connected to: ", url.getBuffer());
  errorCode = httpGet.GET();
  log(CLASS, Info, "Response code to GET: ", errorCode);
  httpGet.writeToStream(&Serial);
  httpGet.end();

#endif // UNIT_TEST

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

void Messenger::getInfo(int infoIndex, Buffer<MAX_VALUE_STR_LENGTH>* info) { }

int Messenger::getNroInfos() { return 0; }

FreqConf *Messenger::getFrequencyConfiguration() { return &freqConf; }
