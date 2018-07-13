#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

void setup() {
  delay(2 * 1000);
  Serial.begin(115200);
  delay(1000);
  Serial.printf("Startup\n");
}

bool initWifi(const char *ssid, const char *pass, int retries) {
  wl_status_t status;
  Serial.printf("Connect to '%s'/'%s'...\n", ssid, pass);

  Serial.printf("W.Off.\n");
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF); // to be removed after SDK update to 1.5.4
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  int attemptsLeft = retries;
  while (true) {
    delay(1000);
    status = WiFi.status();
    Serial.printf(" ..retry(%d)\n", attemptsLeft);
    attemptsLeft--;
    if (status == WL_CONNECTED) {
      Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
      return true; // connected
    }
    if (attemptsLeft < 0) {
      Serial.printf("Conn. failed %d\n", status);
      return false; // not connected
    }
  }
}

void loop() {
  bool connected = initWifi("aphone", "apassword", 20);
  HTTPClient httpClient;
  const char* url = "http://dweet.io/get/latest/dweet/for/demo2-setupsync-target";
  httpClient.begin(url);
  httpClient.addHeader("Content-Type", "application/json");
  Serial.printf("> GET:..%s\n", url);
  int errorCode = httpClient.GET();
  //int errorCode2 = httpClient.POST(body);
  Serial.printf("> GET:%d\n", errorCode);
  int e = httpClient.writeToStream(&Serial);
  Serial.printf("> GET(%d):%d %s\n", e, errorCode, httpClient.errorToString(errorCode).c_str());
  httpClient.end();
  delay(1000);

}
