#include <actors/Messenger.h>
#include <actors/ParamStream.h>

#define CLASS "Messenger"

#define WIFI_SSID "Lola"
#define WIFI_PASSWORD "yourpassword"
#define URL "http://10.0.0.16:9000/dev/0/status"
#define DELAY_UNIT_MS 5000

Messenger::Messenger(const char* n): freqConf(OnceEvery5Minutes) {
  name = n;
  bot = NULL;
}

void Messenger::setBot(Bot* b) {
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

  char configs[100]; configs[0] = 0;
  bot->getProps(configs);

  ParamStream s;
#ifndef UNIT_TEST
  HTTPClient http;
  http.begin(URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Auth-Token", TOKEN);
  log(CLASS, Info, "Client connected to: ", URL);
  log(CLASS, Info, "Putting: ", configs);
  int errorCode = http.POST(configs);
  log(CLASS, Info, "Response code: ", errorCode);
  http.writeToStream(&Serial);
  http.end();
#endif // UNIT_TEST

  int available = s.getNroCommandsAvailable();
  for (int i=0; i<available; i++) {
    Command* c = &s.getCommands()[i];
    Integer newValue;
    newValue.load(&c->newValue);
    log(CLASS, Info, "Setting new configurable: ", c->confIndex);
    log(CLASS, Info, "            property    : ", c->propIndex);
    log(CLASS, Info, "            value       : ", c->newValue.getBuffer());
    bot->setProp(c->confIndex, c->propIndex, &newValue);
  }
  s.flush();

}

void Messenger::subCycle(float subCycle) { }

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
