#include <actors/Messenger.h>

#define CLASS "Messenger"

const char* ssid     = "Lola";
const char* password = "yourpassword";
const char* host = "10.0.0.12";

Messenger::Messenger() {

  log(CLASS, Info, "Connecting to ");
  log(CLASS, Info, ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    log(CLASS, Info, ".");
  }

  log(CLASS, Info, "WIFI OK");

}

const char *Messenger::getName() {
  return "hey";
}

void Messenger::cycle(bool cronMatches) {

  delay(5000);

  log(CLASS, Info, "Connecting to ");
  log(CLASS, Info, host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    log(CLASS, Info, "Connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/api/";
  url += "?param1=";
  url += "hey";
  url += "&param2=";
  url += "babe";

  log(CLASS, Info, "Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      log(CLASS, Info, ">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.println(line);
  }


  log(CLASS, Info, "Closing connection");

}

void Messenger::subCycle(float subCycle) { }

int Messenger::getActuatorValue() { return 0; }

void Messenger::setConfig(int configIndex, char *retroMsg, bool set) { }

int Messenger::getNroConfigs() { return 0; }

void Messenger::getInfo(int infoIndex, char *retroMsg) { }

int Messenger::getNroInfos() { return 0; }

FreqConf *Messenger::getFrequencyConfiguration() { return &freqConf; }
