#include <actors/Messenger.h>

#define CLASS "Messenger"

#define WIFI_SSID "Lola"
#define WIFI_PASSWORD "yourpassword"
#define URL "http://10.0.0.8:9000/api/0"
#define DELAY_UNIT_MS 500

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
  int attempts = 0;
  while(WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(DELAY_UNIT_MS);
      log(CLASS, Info, ".");
      if (attempts++ > 100) {
        break;
      }
    }
  }
  delay(DELAY_UNIT_MS);
}

void Messenger::cycle(bool cronMatches) {

  connectToWifi();

  char configs[100]; configs[0] = 0;
  bot->getConfigs(configs);

  HTTPClient http;
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.POST(configs);
  http.writeToStream(&Serial);
  http.end();

}

void Messenger::subCycle(float subCycle) { }

int Messenger::getActuatorValue() { return 0; }

void Messenger::setConfig(int configIndex, char *retroMsg, SetMode set, int* value) { }

int Messenger::getNroConfigs() { return 0; }

void Messenger::getInfo(int infoIndex, char *retroMsg) { }

int Messenger::getNroInfos() { return 0; }

FreqConf *Messenger::getFrequencyConfiguration() { return &freqConf; }
