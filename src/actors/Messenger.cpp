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
#ifndef UBIDOT_URL
#error "Must provide UBIDOT_URL"
#endif
#ifndef UBIDOT_TOKEN
#error "Must provide UBIDOT_TOKEN"
#endif

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

  char configs[MAX_JSON_STR_LENGTH];
  bot->getProps(configs);

#ifndef UNIT_TEST
  HTTPClient http;
  http.begin(UBIDOT_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Auth-Token", UBIDOT_TOKEN);
  log(CLASS, Info, "Client connected to: ", UBIDOT_URL);
  log(CLASS, Info, "Putting: ", configs);
  int errorCode = http.POST(configs);
  log(CLASS, Info, "Response code: ", errorCode);
  http.writeToStream(&Serial);
  http.end();
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
