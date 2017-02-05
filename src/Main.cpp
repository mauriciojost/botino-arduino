#include <Main.h>

#include <ESP8266WiFi.h>

const char* ssid     = "Lola";
const char* password = "yourpassword";

const char* host = "10.0.0.12";

int value = 0;
volatile int toggle;

void handler(void) {
  toggle = (toggle == 1) ? 0 : 1;
  digitalWrite(BUILTIN_LED, toggle);
  timer0_write(ESP.getCycleCount() + 41660000);
}

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WIFI OK");

  pinMode(BUILTIN_LED, OUTPUT);
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(handler);
  timer0_write(ESP.getCycleCount() + 41660000);
  interrupts();

}


void loop() {
  delay(5000);
  ++value;

  Serial.print("Connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/api/";
  url += streamId;
  url += "?param1=";
  url += "hey";
  url += "&param2=";
  url += "babe";
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("Closing connection");
}

