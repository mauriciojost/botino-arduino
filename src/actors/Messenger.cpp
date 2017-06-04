#include <actors/Messenger.h>
#include <actors/ParamStream.h>
#include <main4ino/Clock.h>
#include <main4ino/Boolean.h>

#define CLASS "Messenger"

#define DELAY_UNIT_MS 5000
#define WAIT_BEFORE_REPOST_DWEETIO_MS 1500
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

Messenger::Messenger(const char* n): freqConf(OnceEvery10Seconds) {
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
    updateClockProperties();
    updateBotProperties();
  }
}

void Messenger::updateClockProperties() {
#ifndef UNIT_TEST
  ParamStream s;
  int errorCode;

  HTTPClient httpGet;
  staticBuffer.clear();
  staticBuffer.fill(TIMEZONE_DB_API_URL_BASE_GET, TIMEZONE_DB_KEY, TIMEZONE_DB_ZONE);
  httpGet.begin(staticBuffer.getBuffer());
  log(CLASS, Info, "Client connected to: ", staticBuffer.getBuffer());
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
  s.flush();
#endif // UNIT_TEST
}

void Messenger::setUpDweetClient(HTTPClient* client, Buffer<MAX_URL_EFF_LENGTH> *url) {
  client->begin(url->getBuffer());
  client->addHeader("Content-Type", "application/json");
  client->addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);
  log(CLASS, Info, "Client connected to: ", url->getBuffer());
}

void Messenger::updateBotProperties() {
#ifndef UNIT_TEST
  int errorCode;

  delay(WAIT_BEFORE_REPOST_DWEETIO_MS);

  HTTPClient client;

  ParamStream s;
  staticUrl.clear();
  staticUrl.fill(DWEET_IO_API_URL_BASE_GET, DEVICE_NAME);
  setUpDweetClient(&client, &staticUrl);
  errorCode = client.GET();
  log(CLASS, Info, "Response code to GET: ", errorCode);
  client.writeToStream(&s);
  client.end();

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
  s.flush();

  bot->getPropsJsonFlat(&staticBuffer);
  staticUrl.clear();
  staticUrl.fill(DWEET_IO_API_URL_BASE_POST, DEVICE_NAME);
  log(CLASS, Info, "Post body: ", staticBuffer.getBuffer());

  setUpDweetClient(&client, &staticUrl);
  errorCode = client.POST(staticBuffer.getBuffer());
  log(CLASS, Info, "Response code to POST: ", errorCode);
  client.writeToStream(&Serial);
  client.end();

  delay(WAIT_BEFORE_REPOST_DWEETIO_MS);

#endif // UNIT_TEST
}


void Messenger::getActuatorValue(Value* value) { }

void Messenger::setProp(int propIndex, SetMode set, const Value* targetValue, Value* actualValue) { }

int Messenger::getNroProps() { return 0; }

const char* Messenger::getPropName(int propIndex) {
  return "";
}

void Messenger::getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH>* info) { }

int Messenger::getNroInfos() { return 0; }

FreqConf *Messenger::getFrequencyConfiguration() { return &freqConf; }
