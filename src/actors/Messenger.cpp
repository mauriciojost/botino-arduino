#include <actors/Messenger.h>

#define CLASS "Messenger"

const char* ssid     = "Lola";
const char* password= "yourpassword";
const char* host = "10.0.0.8";
const int port = 80;

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

void Messenger::cycle(bool cronMatches) {

  static bool initialized = false;
  if (!initialized) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      log(CLASS, Info, ".");
    }
    initialized = true;
    delay(1000);
  }

  char configs[100]; configs[0] = 0;
  bot->getConfigs(configs);

  HTTPClient http;
  http.begin("http://jsonplaceholder.typicode.com/posts");
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
