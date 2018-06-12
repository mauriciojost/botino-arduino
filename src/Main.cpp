/**
 * This file aims to be the only HW specific source code file in the whole project.
 * The rest of the classes (and the 100% of their code) should be testeable without need
 * of Arduino specific HW.
 */

#define CLASS_MAIN "MA"

#include <Module.h>
#include "Images.h"
#include "main4ino/Misc.h"

#ifndef SIMULATE // ESP8266

#include "EspSaveCrash.h"
#include "RemoteDebug.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Pinout.h>
#include <SPI.h>
#include <Servo.h>
#include <Wire.h>

#ifndef PROJ_VERSION
#define PROJ_VERSION "1master"
#endif // PROJ_VERSION

#ifndef WIFI_SSID_INIT
#error "Must provide WIFI_SSID_INIT"
#define WIFI_SSID_INIT ""
#endif

#ifndef WIFI_PASSWORD_INIT
#error "Must provide WIFI_PASSWORD_INIT"
#define WIFI_PASSWORD_INIT ""
#endif

extern "C" {
#include "user_interface.h"
}

#else // SIMULATE (on PC)

#define CL_MAX_LENGTH 1000
#define CURL_COMMAND_GET "curl --silent -XGET '%s'"
#define CURL_COMMAND_POST "curl --silent -XPOST '%s' -d '%s'"

#endif // SIMULATE

#define DELAY_MS_SPI 3
#define FRAG_TO_SLEEP_MS_MAX 2000 // maximum sleeping time for which the module can be unresponsive

#ifndef SERVO0_INVERTED
#define SERVO0_INVERTED false
#endif // SERVO0_INVERTED

#ifndef SERVO1_INVERTED
#define SERVO1_INVERTED false
#endif // SERVO1_INVERTED

#define MAX_SERVO_STEPS 10

#ifndef SERVO0_BASE_DEGREES
#define SERVO0_BASE_DEGREES 10
#endif // SERVO0_BASE_DEGREES

#ifndef SERVO1_BASE_DEGREES
#define SERVO1_BASE_DEGREES 10
#endif // SERVO1_BASE_DEGREES

#ifndef SERVO0_RANGE_DEGREES
#define SERVO0_RANGE_DEGREES 140
#endif // SERVO0_RANGE_DEGREES

#ifndef SERVO1_RANGE_DEGREES
#define SERVO1_RANGE_DEGREES 140
#endif // SERVO1_RANGE_DEGREES

#ifndef PERIOD_SEC
#define PERIOD_SEC 60
#endif // PERIOD_SEC

#define PERIOD_MSEC (PERIOD_SEC * 1000)

#define COMMAND_MAX_LENGTH 64

#define SERVO0_STEP_DEGREES (SERVO0_RANGE_DEGREES / MAX_SERVO_STEPS)
#define SERVO1_STEP_DEGREES (SERVO1_RANGE_DEGREES / MAX_SERVO_STEPS)

#ifndef DWEET_IO_API_TOKEN
#error "Must provide DWEET_IO_API_TOKEN"
#endif

#define WAIT_BEFORE_HTTP_MS 1500

#define SERVO_INVERT_POS(p, f) (((f) ? (180 - (p)) : (p)))

#define URL_PRINT_MAX_LENGTH 20

Module m;
volatile unsigned char ints = 0;

// clang-format off

uint8_t initImage[IMG_SIZE_BYTES] = {0b00000000, 0b00000000,
                                     0b01111110, 0b00000000,
                                     0b01000010, 0b00111110,
                                     0b01000010, 0b00100010,
                                     0b01001010, 0b00101010,
                                     0b01000010, 0b00100010,
                                     0b01111110, 0b00111110,
                                     0b00000000, 0b00000000};

// clang-format on

#ifndef SIMULATE // ESP8266

HTTPClient httpClient;
RemoteDebug Telnet;
Servo servoLeft;
Servo servoRight;
Adafruit_SSD1306 lcd(-1);


/********************/
/*** HW FUNCTIONS ***/
/********************/

void arms(int left, int right, int steps);
void lcdImg(char img, uint8_t bitmap[]);
void ios(char led, bool v);
void messageOnLcd(int line, const char *str, int size);


void lcdClear(int line) {
  lcd.fillRect(0, line * 8, 128, 8, BLACK);
}

void lcdHighlight(int line) {
  int offset;
  offset = 0;
  lcd.fillRect(0, line * 8 + offset, 128, 8 - offset, BLACK);
  offset = 2;
  lcd.fillRect(0, line * 8 + offset, 128, 8 - offset, WHITE);
  offset = 4;
  lcd.fillRect(0, line * 8 + offset, 128, 8 - offset, BLACK);
  offset = 6;
  lcd.fillRect(0, line * 8 + offset, 128, 8 - offset, WHITE);
}

void lcdPrintLogLine(const char *logStr) {
  if (!m.getSettings()->getLcdDebug()) {
    return;
  }
  static int line = 0;
  lcd.setTextWrap(false);
  lcdClear(line); // clear line
  lcd.setTextSize(1);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, line * 8);
  lcd.println(logStr);
  line = (line + 1) % 8;
  lcdHighlight(line); // clear next line too (to indicate move)
  lcd.display();
  delay(DELAY_MS_SPI);
}

void displayUserInfo() {
  Buffer<32> aux;
  log(CLASS_MAIN, Debug, "USER INFO");
  aux.fill("VER: %s", PROJ_VERSION);
  messageOnLcd(0, aux.getBuffer(), 2);
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(3000);
  aux.fill("NAM: %s", DEVICE_NAME);
  messageOnLcd(0, aux.getBuffer(), 2);
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(3000);
  aux.fill("ID : %d", ESP.getChipId());
  messageOnLcd(0, aux.getBuffer(), 2);
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(3000);
  aux.fill("SSI: %s", WIFI_SSID_INIT);
  messageOnLcd(0, aux.getBuffer(), 2);
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(3000);
  aux.fill("PAS: %s", WIFI_PASSWORD_INIT);
  messageOnLcd(0, aux.getBuffer(), 2);
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(3000);
}

void bitmapToLcd(uint8_t bitmap[]) {
  for (char yi = 0; yi < 8; yi++) {
    for (char xi = 0; xi < 2; xi++) {
      uint8_t imgbyte = bitmap[yi * 2 + xi];
      for (char b = 0; b < 8; b++) {
        uint8_t color = (imgbyte << b) & 0b10000000;
        int16_t xl = (int16_t)xi * 64 + (int16_t)b * 8;
        int16_t yl = (int16_t)yi * 8;
        uint16_t cl = color == 0 ? BLACK : WHITE;
        lcd.fillRect(xl, yl, 8, 8, cl);
      }
    }
  }
}

#else // SIMULATE (on PC)
// no HW functions replacement (as they won't be called)
#endif // SIMULATE


bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries) {
#ifndef SIMULATE // ESP8266
  wl_status_t status;
  log(CLASS_MAIN, Info, "Conn. to %s...", ssid);

  if (skipIfConnected) {
    log(CLASS_MAIN, Info, "Conn.?");
    status = WiFi.status();
    if (status == WL_CONNECTED) {
      log(CLASS_MAIN, Info, "IP: %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
  } else {
    log(CLASS_MAIN, Info, "W.Off.");
    WiFi.mode(WIFI_OFF); // to be removed after SDK update to 1.5.4
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  int attemptsLeft = retries;
  while (true) {
    delay(3000);
    status = WiFi.status();
    log(CLASS_MAIN, Info, " ..retry(%d)", attemptsLeft);
    attemptsLeft--;
    if (status == WL_CONNECTED) {
      log(CLASS_MAIN, Info, "IP: %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
    if (attemptsLeft < 0) {
      log(CLASS_MAIN, Warn, "Conn. failed %d", status);
      return false; // not connected
    }
  }
#else // SIMULATE (on PC)
  log(CLASS_MAIN, Debug, "initWifi(%s, %s, %d)", ssid, pass, retries);
  return true;
#endif // SIMULATE
}

void handleSettings() {
  Settings *s = m.getSettings();

#ifndef SIMULATE // ESP8266
  // Handle stack-traces stored in memory
  if (s->getClear() && SaveCrash.count() > 0) {
    log(CLASS_MAIN, Debug, "Clearing stack-trcs");
    SaveCrash.clear();
  } else if (SaveCrash.count() > 0) {
    log(CLASS_MAIN, Warn, "Stack-trcs (!!!)");
    SaveCrash.print();
  }
#else // SIMULATE (on PC)
  // noting to do here
#endif // SIMULATE

  // Handle log level as per settings
  setLogLevel((char)(s->getLogLevel() % 4));
}

void handleServices() {
#ifndef SIMULATE // ESP8266
  // Handle telnet log server
  Telnet.handle();

  // Handle OTA
  ArduinoOTA.handle();
#else // SIMULATE (on PC)
  // noting to do here
#endif // SIMULATE
}

void reactCommand(const char* cmd) {
  char command[COMMAND_MAX_LENGTH];
  strncpy(command, cmd, COMMAND_MAX_LENGTH);
  log(CLASS_MAIN, Info, "Command: %s", command);

  char *c = strtok(command, " ");

  if (strcmp("conf", c) == 0) {
    log(CLASS_MAIN, Info, "-> Conf mode");
    m.getBot()->setMode(ConfigureMode);
    return;
  } else if (strcmp("move", c) == 0) {
    c = strtok(NULL, " ");
    if (c == NULL) {
      log(CLASS_MAIN, Error, "Argument needed:\n  move <move>");
      return;
    }
    log(CLASS_MAIN, Info, "-> Move %s", c);
    m.getBody()->performMove(c);
    return;
  } else if (strcmp("set", c) == 0) {
    const char *actor = strtok(NULL, " ");
    const char *prop = strtok(NULL, " ");
    const char *v = strtok(NULL, " ");
    if (actor == NULL || prop == NULL || v == NULL) {
      log(CLASS_MAIN, Error, "Arguments needed:\n  set <actor> <prop> <value>");
      return;
    }
    log(CLASS_MAIN, Info, "-> Set %s.%s = %s", actor, prop, v);
    Buffer<64> value(v);
    m.getBot()->setProp(actor, prop, &value);
    return;
  } else if (strcmp("get", c) == 0) {
    log(CLASS_MAIN, Info, "-> Get");
    Array<Actor *> *actors = m.getBot()->getActors();
    for (int i = 0; i < actors->size(); i++) {
      Actor *actor = actors->get(i);
      log(CLASS_MAIN, Info, " '%s'", actor->getName());
      for (int j = 0; j < actor->getNroProps(); j++) {
        Buffer<COMMAND_MAX_LENGTH> value;
        actor->getPropValue(j, &value);
        log(CLASS_MAIN, Info, "   '%s': '%s'", actor->getPropName(j), value.getBuffer());
      }
      log(CLASS_MAIN, Info, " ");
    }
    log(CLASS_MAIN, Info, " ");
    return;
  } else if (strcmp("run", c) == 0) {
    log(CLASS_MAIN, Info, "-> Run mode");
    m.getBot()->setMode(RunMode);
    return;
  } else {
    log(CLASS_MAIN, Error, "Invalid command (try: ?)");
    return;
  }
}

#ifndef SIMULATE // ESP8266
void reactCommandCustom() {
	reactCommand(Telnet.getLastCommand().c_str());
}
#endif // SIMULATE

void reactButton() {
#ifndef SIMULATE // ESP8266
  delay(100); // avoid bouncing
  int level = digitalRead(BUTTON0_PIN);
  if (ints > 0 && level) {
    log(CLASS_MAIN, Debug, "Btn.hold(%d)...", ints);
    log(CLASS_MAIN, Debug, "Done (%d)", ints);
  } else if (ints > 0 && !level) { // pressed the button, but not currently being pressed
    log(CLASS_MAIN, Debug, "Btn.quick(%d)...", ints);
    int routine = (int)random(0, NRO_ROUTINES);
    log(CLASS_MAIN, Debug, "Routine %d...", routine);
    m.getBody()->performMove(routine);
    log(CLASS_MAIN, Debug, "Done (%d)", ints);
  }
  digitalWrite(LEDW_PIN, HIGH);
  ints = 0;
#else // SIMULATE (on PC)
  // noting to do here
#endif // SIMULATE
}

#ifndef SIMULATE // ESP8266
void performHardwareTest() {
  log(CLASS_MAIN, Debug, "HW test");
  delay(2000);
  ios('r', false);
  ios('y', false);
  ios('w', false);
  ios('f', false);
  lcdImg('l', NULL);

  log(CLASS_MAIN, Debug, "..Face test");
  delay(2000);
  lcdImg('c', initImage);
  log(CLASS_MAIN, Debug, "..Arms down");
  delay(2000);
  arms(0, 0, 100);
  log(CLASS_MAIN, Debug, "..R. arm up");
  delay(2000);
  arms(0, 3, 100);
  log(CLASS_MAIN, Debug, "..Left arm up");
  delay(2000);
  arms(3, 3, 100);
  log(CLASS_MAIN, Debug, "..Arms down");
  delay(2000);
  arms(0, 0, 100);
  log(CLASS_MAIN, Debug, "..Red led on");
  delay(2000);
  ios('r', true);
  log(CLASS_MAIN, Debug, "..Red led off");
  delay(2000);
  ios('r', false);
  log(CLASS_MAIN, Debug, "..Y. led on");
  delay(2000);
  ios('y', true);
  log(CLASS_MAIN, Debug, "..Y. led off");
  delay(2000);
  ios('y', false);
  log(CLASS_MAIN, Debug, "..W. led on");
  delay(2000);
  ios('w', true);
  log(CLASS_MAIN, Debug, "..W. led off");
  delay(2000);
  ios('w', false);
  log(CLASS_MAIN, Debug, "..Fan on");
  delay(2000);
  ios('f', true);
  log(CLASS_MAIN, Debug, "..Fan off");
  delay(2000);
  ios('f', false);
  log(CLASS_MAIN, Debug, "..Random %lu %lu %lu", random(10000), random(10000), random(10000));
  delay(2000);
}

#else // SIMULATE (on PC)

// nothing here to replace hardware funcitons on
// the PC

#endif  // SIMULATE


/*****************/
/*** CALLBACKS ***/
/*****************/

#ifndef SIMULATE // ESP8266
ICACHE_RAM_ATTR
void buttonPressed() {
  ints++;
  digitalWrite(LEDW_PIN, LOW);
}
#endif // SIMULATE

void messageOnLcd(int line, const char *str, int size) {
#ifndef SIMULATE // ESP8266
  lcd.clearDisplay();
  lcd.setTextWrap(true);
  lcd.setTextSize(size);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, line * 2 * 8);
  lcd.println(str);
  lcd.display();
  delay(DELAY_MS_SPI);
#else // SIMULATE (on PC)
  printf("LCD: %s (size %d)", str, size);
#endif // SIMULATE
}

void logLine(const char *str) {
#ifndef SIMULATE // ESP8266
  lcdPrintLogLine(str);
  Serial.println(str);
  Telnet.printf("%s\n", str);
#else // SIMULATE (on PC)
  printf("LOG: %s", str);
#endif // SIMULATE
}

void arms(int left, int right, int steps) {
#ifndef SIMULATE // ESP8266
  static int lastPosL = -1;
  static int lastPosR = -1;

  log(CLASS_MAIN, Debug, "Arms>%d&%d", left, right);
  int targetPosL = SERVO_INVERT_POS(((POSIT(left) % MAX_SERVO_STEPS) * SERVO0_STEP_DEGREES) + SERVO0_BASE_DEGREES, SERVO0_INVERTED);
  int targetPosR = SERVO_INVERT_POS(((POSIT(right) % MAX_SERVO_STEPS) * SERVO1_STEP_DEGREES) + SERVO1_BASE_DEGREES, SERVO1_INVERTED);
  servoLeft.attach(SERVO0_PIN);
  servoRight.attach(SERVO1_PIN);

  // leave as target if first time
  lastPosL = (lastPosL == -1 ? targetPosL : lastPosL);
  lastPosR = (lastPosR == -1 ? targetPosR : lastPosR);

  log(CLASS_MAIN, Debug, "Sv.L%d>%d", lastPosL, targetPosL);
  log(CLASS_MAIN, Debug, "Sv.R%d>%d", lastPosR, targetPosR);
  for (int i = 1; i <= steps; i++) {
    float factor = ((float)i) / steps;
    int vL = lastPosL + ((targetPosL - lastPosL) * factor);
    int vR = lastPosR + ((targetPosR - lastPosR) * factor);
    servoLeft.write(vL);
    servoRight.write(vR);
    delay(15);
  }
  lastPosL = targetPosL;
  lastPosR = targetPosR;
  servoLeft.detach();
  servoRight.detach();
#else // SIMULATE (on PC)
  printf("ARMS: %d %d %d", left, right, steps);
#endif // SIMULATE
}

bool initWifiInit() {
  log(CLASS_PROPSYNC, Info, "W.init %s", WIFI_SSID_INIT);
  return initWifi(WIFI_SSID_INIT, WIFI_PASSWORD_INIT, false, 20);
}

bool initWifiSteady() {
  SetupSync *s = m.getSetupSync();
  static bool connectedOnce = false;
  if (s->isInitialized()) {
    const char *wifiSsid = s->getSsid();
    const char *wifiPass = s->getPass();
    log(CLASS_PROPSYNC, Info, "W.steady %s", wifiSsid);
    bool connected = initWifi(wifiSsid, wifiPass, connectedOnce, 5);
    if (!connectedOnce) {
      messageOnLcd(0, "WIFI SETUP...", 2);
      delay(1 * 2000);
      messageOnLcd(0, wifiSsid, 2);
      delay(1 * 2000);
      if (connected) { // first time
        messageOnLcd(0, "SETUP OK", 2);
        log(CLASS_MAIN, Info, "SETUP OK");
        delay(10 * 1000);
      }
    }
    connectedOnce = connectedOnce || connected;
    return connected;
  } else {
    log(CLASS_PROPSYNC, Info, "W.steady not ready");
    return false;
  }
}

int httpGet(const char *url, ParamStream *response) {
#ifndef SIMULATE // ESP8266
  httpClient.begin(url);
  httpClient.addHeader("Content-Type", "application/json");
  httpClient.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);

  int errorCode = httpClient.GET();
  log(CLASS_MAIN, Debug, "GET:%d..%s", errorCode, tailStr(url, URL_PRINT_MAX_LENGTH));

  if (errorCode > 0) {
    if (response != NULL) {
      httpClient.writeToStream(response);
    }
  } else {
    log(CLASS_MAIN, Error, "GET:%d %s", errorCode, httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
#else // SIMULATE (on PC)
  Buffer<CL_MAX_LENGTH> aux;
  aux.fill(CURL_COMMAND_GET, url);
  log(CLASS_MAIN, Debug, "GET: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {return -1;}
  while (fgets(aux.getUnsafeBuffer(), 1000 -1, fp) != NULL) {
    response->fill(aux.getBuffer());
  }
  log(CLASS_MAIN, Debug, "-> %s", response->content());
  pclose(fp);
  return 1;
#endif // SIMULATE
}

int httpPost(const char *url, const char *body, ParamStream *response) {
#ifndef SIMULATE // ESP8266
  httpClient.begin(url);
  httpClient.addHeader("Content-Type", "application/json");
  httpClient.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);

  int errorCode = httpClient.POST(body);
  log(CLASS_MAIN, Debug, "POST:%d..%s", errorCode, tailStr(url, URL_PRINT_MAX_LENGTH));

  if (errorCode > 0) {
    if (response != NULL) {
      httpClient.writeToStream(response);
    }
  } else {
    log(CLASS_MAIN, Error, "POST:%d %s", errorCode, httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
#else // SIMULATE (on PC)
  Buffer<CL_MAX_LENGTH> aux;
  aux.fill(CURL_COMMAND_POST, url, body);
  log(CLASS_MAIN, Debug, "POST: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {return -1;}
  while (fgets(aux.getUnsafeBuffer(), CL_MAX_LENGTH -1, fp) != NULL) {
    response->fill(aux.getBuffer());
  }
  log(CLASS_MAIN, Debug, "-> %s", response->content());
  pclose(fp);
  return 1;
#endif // SIMULATE
}

void ios(char led, bool v) {
  log(CLASS_MAIN, Debug, "Led'%c'->%d", led, (int)v);
#ifndef SIMULATE // ESP8266
  switch (led) {
    case 'r':
      digitalWrite(LEDR_PIN, !v); // VCC hard-wired
      break;
    case 'w':
      digitalWrite(LEDW_PIN, !v); // VCC hard-wired
      break;
    case 'y':
      digitalWrite(LEDY_PIN, !v); // VCC hard-wired
      break;
    case 'f':
      digitalWrite(FAN_PIN, v);
      break;
    default:
      break;
  }
#else // SIMULATE (on PC)
  // nothing here, action already logged
#endif // SIMULATE
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "Img '%c'", img);
#ifndef SIMULATE // ESP8266
  switch (img) {
    case 'w': // white
      log(CLASS_BODY, Debug, "White face");
      lcd.invertDisplay(true);
      break;
    case 'b': // black
      log(CLASS_BODY, Debug, "Black face");
      lcd.invertDisplay(false);
      break;
    case 'l': // clear
      log(CLASS_BODY, Debug, "Clear face");
      lcd.clearDisplay();
      break;
    case 'c': // custom
      log(CLASS_BODY, Debug, "Custom face", img);
      if (bitmap != NULL) {
        logHex(CLASS_BODY, Debug, bitmap, IMG_SIZE_BYTES);
        bitmapToLcd(bitmap); // custom
      }
      break;
    default:
      log(CLASS_BODY, Debug, "Face?: %c", img);
      break;
  }
  lcd.display();
  delay(DELAY_MS_SPI);
#else // SIMULATE (on PC)
  // nothing here, action already logged
#endif // SIMULATE
}

/*****************/
/***** SETUP *****/
/*****************/

void setup() {

#ifndef SIMULATE // ESP8266
  // Let HW startup
  delay(2 * 1000);

  // Intialize the logging framework
  Serial.begin(115200);            // Initialize serial port
  Telnet.begin("ESP" DEVICE_NAME); // Intialize the remote logging framework
  ArduinoOTA.begin();              // Intialize OTA
  lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(DELAY_MS_SPI); // Initialize LCD
  setupLog(logLine);   // Initialize log callback

  log(CLASS_MAIN, Debug, "Setup pins");
  pinMode(LEDR_PIN, OUTPUT);
  pinMode(LEDW_PIN, OUTPUT);
  pinMode(LEDY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(SERVO0_PIN, OUTPUT);
  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(BUTTON0_PIN, INPUT);

  log(CLASS_MAIN, Debug, "Setup random");
  randomSeed(analogRead(0) * 256 + analogRead(0));

#else // SIMULATE (on PC)
  // nothing here
#endif // SIMULATE

  // TODO all this should be wrapped by the Module
  log(CLASS_MAIN, Debug, "Setup module");
  m.getBody()->setLcdImgFunc(lcdImg);
  m.getBody()->setArmsFunc(arms);
  m.getBody()->setMessageFunc(messageOnLcd);
  m.getBody()->setIosFunc(ios);
  m.getPropSync()->setInitWifi(initWifiSteady);
  m.getPropSync()->setHttpPost(httpPost);
  m.getPropSync()->setHttpGet(httpGet);
  m.getClockSync()->setInitWifi(initWifiSteady);
  m.getClockSync()->setHttpGet(httpGet);
  m.getSetupSync()->setInitWifiSteady(initWifiSteady);
  m.getSetupSync()->setInitWifiInit(initWifiInit);
  m.getSetupSync()->setHttpGet(httpGet);
  m.getQuotes()->setHttpGet(httpGet);
  m.getQuotes()->setInitWifi(initWifiSteady);

#ifndef SIMULATE // ESP8266

  log(CLASS_MAIN, Debug, "Setup interrupts");
  attachInterrupt(digitalPinToInterrupt(BUTTON0_PIN), buttonPressed, RISING);

  log(CLASS_MAIN, Debug, "Setup commands");
  Telnet.setCallBackProjectCmds(reactCommandCustom);
  String helpCli("\n  conf   : go to configuration mode"
                 "\n  run    : go to run mode"
                 "\n  get    : display actors properties"
                 "\n  set    : set an actor property (example: 'set body msg0 HELLO')"
                 "\n  move   : execute a move (example: 'move A00C55')"
                 "\n");
  Telnet.setHelpProjectsCmds(helpCli);

  if (digitalRead(BUTTON0_PIN) == HIGH) {
    log(CLASS_MAIN, Info, "CLI mode");
    messageOnLcd(0, "CLI mode", 2);
    delay(1000);
    initWifiSteady();
    m.getBot()->setMode(ConfigureMode);
    log(CLASS_MAIN, Info, "telnet");
    log(CLASS_MAIN, Info, "  %s", WiFi.localIP().toString().c_str());
  } else {
    log(CLASS_MAIN, Info, "REGULAR mode");
    messageOnLcd(0, "REGULAR mode", 2);
    delay(1000);
    displayUserInfo();
#ifdef HARDWARE_TEST
    performHardwareTest();
#endif
  }
#else // SIMULATE (on PC)
  // something to reactCommand
#endif // SIMULATE
}

/**
 * Sleep the remaining part of the cycle.
 */
void sleepInCycle(unsigned long cycleBegin) {
  log(CLASS_MAIN, Info, "L.Sleep(%lums)...", PERIOD_MSEC);
  unsigned long spentMs = millis() - cycleBegin;
  log(CLASS_MAIN, Info, "D.C.:%0.3f", (float)spentMs / PERIOD_MSEC);
  while (spentMs < PERIOD_MSEC) {
    reactButton();
    unsigned long fragToSleepMs = MINIM(PERIOD_MSEC - spentMs, FRAG_TO_SLEEP_MS_MAX);
#ifndef SIMULATE // ESP8266
    wifi_set_sleep_type(LIGHT_SLEEP_T);
#else // SIMULATE (on PC)
  // nothing here, already logged
#endif // SIMULATE
    delay(fragToSleepMs);
    spentMs = millis() - cycleBegin;
  }

}

void loop() {
  unsigned long t1 = millis();

  handleSettings();

  handleServices();

  switch (m.getBot()->getMode()) {
    case (RunMode):
      m.loop(false, false, true);
      sleepInCycle(t1);
      break;
    case (ConfigureMode):
      break;
    default:
      m.getBot()->setMode(RunMode);
      break;
  }
}

#ifdef SIMULATE
int main( int argc, const char* argv[] ) {
	setup();
	while(true) {
    loop();
	}
}
#endif // SIMULATE
