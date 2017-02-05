#ifndef UNIT_TEST
#include <Main.h>

#include <ESP8266WiFi.h>
#include <Module.h>

#define CLASS "Main"

#define BUTTON_DEBOUNCING_DELAY_MS 100
#define LEVEL_VCC_MEASURE_DELAY_MS 10


volatile bool buttonModeWasPressed = false; // flag related to mode button pressed
volatile bool buttonSetWasPressed = false;  // flag related to set button pressed

volatile char nroInterruptsQueued = 0; // counter to keep track of amount of timing
                                       // interrupts queued

volatile unsigned char subCycle = 0; // counter to determine which interrupt is a cycle
                                     // and which are in the middle of a cycle

Module m;


const char* ssid     = "Lola";
const char* password = "yourpassword";

const char* host = "10.0.0.12";

int value = 0;
volatile int toggle;

/*****************/
/** INTERRUPTS ***/
/*****************/

void timingInterrupt(void) {
  toggle = (toggle == 1) ? 0 : 1;
  digitalWrite(BUILTIN_LED, toggle);
  timer0_write(ESP.getCycleCount() + 41660000);

  nroInterruptsQueued++; // increment the queue so the interrupts are treated sometime
}

void ISR_ButtonMode() {
  buttonModeWasPressed = true;
}

void ISR_ButtonSet() {
  buttonSetWasPressed = true;
}

/******************/
/***  CALLBACKS ***/
/******************/

void displayOnLcdString(const char *str1, const char *str2) {
  log(CLASS, Info, str1);
  log(CLASS, Info, str2);
}

/*****************/
/***** SETUP *****/
/*****************/

void setupPins() {
  //pinMode(SERVO_PIN, OUTPUT);
  //attachInterrupt(digitalPinToInterrupt(BUTTON_MODE_PIN), ISR_ButtonMode, RISING);
  //attachInterrupt(digitalPinToInterrupt(BUTTON_SET_PIN), ISR_ButtonSet, RISING);
}


void setup() {
  setupLog();
  delay(1000);
  log(CLASS, Info, "LOG READY");
/*
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WIFI OK");
  */

  setupPins();
  m.setup();
  //m.setStdoutWriteFunction(displayOnLcdString);

  pinMode(BUILTIN_LED, OUTPUT);
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(timingInterrupt);
  timer0_write(ESP.getCycleCount() + 41660000);
  interrupts();

  //m.setReadLevelFunction(readLevel);

}


void loop() {
  log(CLASS, Info, "LOOP");
/*
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

*/
  ///////////

/*
  bool bModeStable = buttonModeWasPressed;// && digitalRead(BUTTON_MODE_PIN);
  bool bSetStable = buttonSetWasPressed;// && digitalRead(BUTTON_SET_PIN);
  bool wdtInterrupt = nroInterruptsQueued > 0;

  if (wdtInterrupt) {
    nroInterruptsQueued--;
  }

  m.loop(bModeStable, bSetStable, wdtInterrupt);
  m.getClock()->setNroInterruptsQueued(nroInterruptsQueued);

  if (buttonSetWasPressed) {
    // smth
  }

  if (buttonModeWasPressed || buttonSetWasPressed) {
    delay(BUTTON_DEBOUNCING_DELAY_MS);
    buttonModeWasPressed = false;
    // disable set button only if no longer being pressed
    //if (!digitalRead(BUTTON_SET_PIN)) {
      //buttonSetWasPressed = false;
    //}
  }

  if (nroInterruptsQueued <= 0) { // no interrupts queued
    nroInterruptsQueued = 0;
    //enterSleep();
  }
  */
  delay(1000);
}

#endif // UNIT_TEST
