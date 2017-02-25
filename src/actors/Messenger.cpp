#include <actors/Messenger.h>
#include <actors/ParamStream.h>

#define CLASS "Messenger"

#define WIFI_SSID "Lola"
#define WIFI_PASSWORD "yourpassword"
#define URL "http://10.0.0.8:9000/dev/0"
#define DELAY_UNIT_MS 5000

Messenger::Messenger() {
  freqConf.setFrequency(OnceEvery5Minutes);
  bot = NULL;
}

void Messenger::setBot(Bot* b) {
  bot = b;
}

const char *Messenger::getName() {
  return CLASS;
}

void Messenger::connectToWifi() {
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
}

void Messenger::cycle(bool cronMatches) {

  connectToWifi();

  char configs[100]; configs[0] = 0;
  bot->getConfigs(configs);

  ParamStream s;
  HTTPClient http;
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  log(CLASS, Info, "Client connected to: ", URL);
  log(CLASS, Info, "Posting: ", configs);
  int errorCode = http.POST(configs);
  log(CLASS, Info, "Response code: ", errorCode);
  http.writeToStream(&s);
  http.end();

  int available = s.getNroCommandsAvailable();
  for (int i=0; i<available; i++) {
    Command* c = &s.getCommands()[i];
    log(CLASS, Info, "Setting new configurable: ", c->configurableIndex);
    log(CLASS, Info, "            property    : ", c->propertyIndex);
    log(CLASS, Info, "            value       : ", c->newValue);
    bot->setConfig(c->configurableIndex, c->propertyIndex, c->newValue);
  }
  s.flush();

}

void Messenger::subCycle(float subCycle) { }

int Messenger::getActuatorValue() { return 0; }

void Messenger::setConfig(int configIndex, char *retroMsg, SetMode set, int* value) { }

int Messenger::getNroConfigs() { return 0; }

void Messenger::getInfo(int infoIndex, char *retroMsg) { }

int Messenger::getNroInfos() { return 0; }

FreqConf *Messenger::getFrequencyConfiguration() { return &freqConf; }
