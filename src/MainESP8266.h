
#include <Main.h>

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

#define HARDWARE_TEST_STEP_DELAY_MS 2000
#define DUTY_CYCLE_THRESHOLD_PERC 50

#define SERVO0_STEP_DEGREES (SERVO0_RANGE_DEGREES / MAX_SERVO_STEPS)
#define SERVO1_STEP_DEGREES (SERVO1_RANGE_DEGREES / MAX_SERVO_STEPS)

#define SERVO_INVERT_POS(p, f) (((f) ? (180 - (p)) : (p)))

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

HTTPClient httpClient;
RemoteDebug Telnet;
Servo servoLeft;
Servo servoRight;
Adafruit_SSD1306 lcd(-1);

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
  lcd.fillRect(0, line * 8 + offset, 128, 8 - offset, BLACK);
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
  messageFunc(0, aux.getBuffer(), 2);
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(3000);
  aux.fill("NAM: %s", DEVICE_NAME);
  messageFunc(0, aux.getBuffer(), 2);
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(3000);
  aux.fill("ID : %d", ESP.getChipId());
  messageFunc(0, aux.getBuffer(), 2);
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(3000);
  aux.fill("SSI: %s", WIFI_SSID_INIT);
  messageFunc(0, aux.getBuffer(), 2);
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(3000);
  aux.fill("PAS: %s", WIFI_PASSWORD_INIT);
  messageFunc(0, aux.getBuffer(), 2);
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

bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries) {
  wl_status_t status;
  log(CLASS_MAIN, Info, "To '%s'/'%s'...", ssid, pass);

  if (skipIfConnected) {
    log(CLASS_MAIN, Info, "Conn.?");
    status = WiFi.status();
    if (status == WL_CONNECTED) {
      log(CLASS_MAIN, Info, "IP: %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
  } else {
    log(CLASS_MAIN, Info, "W.Off.");
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF); // to be removed after SDK update to 1.5.4
    delay(3000);
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
}

void loopArchitecture() {
  Settings *s = m.getSettings();

  // Handle stack-traces stored in memory
  if (s->getClear() && SaveCrash.count() > 0) {
    log(CLASS_MAIN, Debug, "Clearing stack-trcs");
    SaveCrash.clear();
  } else if (SaveCrash.count() > 0) {
    log(CLASS_MAIN, Warn, "Stack-trcs (!!!)");
    SaveCrash.print();
  }

  // Handle log level as per settings
  setLogLevel((char)(s->getLogLevel() % 4));

  // Handle telnet log server
  Telnet.handle();

  // Handle OTA
  ArduinoOTA.handle();
}

void reactCommandCustom() {
  reactCommand(Telnet.getLastCommand().c_str());
}

bool haveToInterrupt() {
  delay(100); // avoid bouncing
  int level = digitalRead(BUTTON0_PIN);
  if (ints > 0 && level) {
    log(CLASS_MAIN, Debug, "Btn.hold(%d)...", ints);
    log(CLASS_MAIN, Debug, "Done (%d)", ints);
  } else if (ints > 0 && !level) { // pressed the button, but not currently being pressed
    log(CLASS_MAIN, Debug, "Btn.quick(%d)...", ints);
    int routine = (int)random(0, m.getSettings()->getNroRoutinesForButton());
    log(CLASS_MAIN, Debug, "Routine %d...", routine);
    m.getBody()->performMove(routine);
    log(CLASS_MAIN, Debug, "Done (%d)", ints);
  }
  digitalWrite(LEDW_PIN, HIGH);
  ints = 0;
  return false;
}

void performHardwareTest() {
  log(CLASS_MAIN, Debug, "HW test");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('r', false);
  ios('y', false);
  ios('w', false);
  ios('f', false);
  lcdImg('l', NULL);

  log(CLASS_MAIN, Debug, "..Face test");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  lcdImg('c', initImage);
  log(CLASS_MAIN, Debug, "..Arms down");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  arms(0, 0, 100);
  log(CLASS_MAIN, Debug, "..R. arm up");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  arms(0, 3, 100);
  log(CLASS_MAIN, Debug, "..Left arm up");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  arms(3, 3, 100);
  log(CLASS_MAIN, Debug, "..Arms down");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  arms(0, 0, 100);
  log(CLASS_MAIN, Debug, "..Red led on");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('r', true);
  log(CLASS_MAIN, Debug, "..Red led off");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('r', false);
  log(CLASS_MAIN, Debug, "..Y. led on");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('y', true);
  log(CLASS_MAIN, Debug, "..Y. led off");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('y', false);
  log(CLASS_MAIN, Debug, "..W. led on");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('w', true);
  log(CLASS_MAIN, Debug, "..W. led off");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('w', false);
  log(CLASS_MAIN, Debug, "..Fan on");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('f', true);
  log(CLASS_MAIN, Debug, "..Fan off");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('f', false);
  log(CLASS_MAIN, Debug, "..Random %lu %lu %lu", random(10000), random(10000), random(10000));
  delay(HARDWARE_TEST_STEP_DELAY_MS);
}

/*****************/
/*** CALLBACKS ***/
/*****************/

ICACHE_RAM_ATTR
void buttonPressed() {
  ints++;
  digitalWrite(LEDW_PIN, LOW);
}

void messageFunc(int line, const char *str, int size) {
  lcd.clearDisplay();
  lcd.setTextWrap(true);
  lcd.setTextSize(size);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, line * 2 * 8);
  lcd.println(str);
  lcd.display();
  delay(DELAY_MS_SPI);
}

void logLine(const char *str) {
  lcdPrintLogLine(str);
  Serial.println(str);
  Telnet.printf("%s\n", str);
}

void arms(int left, int right, int steps) {
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
}

int httpGet(const char *url, ParamStream *response) {
  httpClient.begin(url);
  httpClient.addHeader("Content-Type", "application/json");
  httpClient.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);

  int errorCode = httpClient.GET();
  log(CLASS_MAIN, Debug, "GET:%d..%s", errorCode, tailStr(url, URL_PRINT_MAX_LENGTH));

  if (errorCode == HTTP_OK) {
    if (response != NULL) {
      httpClient.writeToStream(response);
    }
  } else {
    log(CLASS_MAIN, Error, "GET:%d %s", errorCode, httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
}

int httpPost(const char *url, const char *body, ParamStream *response) {
  httpClient.begin(url);
  httpClient.addHeader("Content-Type", "application/json");
  httpClient.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);

  int errorCode = httpClient.POST(body);
  log(CLASS_MAIN, Debug, "POST:%d..%s", errorCode, tailStr(url, URL_PRINT_MAX_LENGTH));

  if (errorCode == HTTP_OK) {
    if (response != NULL) {
      httpClient.writeToStream(response);
    }
  } else {
    log(CLASS_MAIN, Error, "POST:%d %s", errorCode, httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
}

void ios(char led, bool v) {
  log(CLASS_MAIN, Debug, "Led'%c'->%d", led, (int)v);
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
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "Img '%c'", img);
  switch (img) {
    case 'w': // white
      log(CLASS_MAIN, Debug, "White face");
      lcd.invertDisplay(true);
      break;
    case 'b': // black
      log(CLASS_MAIN, Debug, "Black face");
      lcd.invertDisplay(false);
      break;
    case 'l': // clear
      log(CLASS_MAIN, Debug, "Clear face");
      lcd.clearDisplay();
      break;
    case 'c': // custom
      log(CLASS_MAIN, Debug, "Custom face", img);
      if (bitmap != NULL) {
        logHex(CLASS_MAIN, Debug, bitmap, IMG_SIZE_BYTES);
        bitmapToLcd(bitmap); // custom
      }
      break;
    default:
      log(CLASS_MAIN, Debug, "Face?: %c", img);
      break;
  }
  lcd.display();
  delay(DELAY_MS_SPI);
}

void setupArchitecture() {

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
    messageFunc(0, "CLI mode", 2);
    delay(1000);
    initWifiSteady();
    m.getBot()->setMode(ConfigureMode);
    log(CLASS_MAIN, Info, "telnet");
    log(CLASS_MAIN, Info, "  %s", WiFi.localIP().toString().c_str());
  } else {
    log(CLASS_MAIN, Info, "REGULAR mode");
    messageFunc(0, "REGULAR mode", 2);
    delay(1000);
    displayUserInfo();
#ifdef HARDWARE_TEST
    performHardwareTest();
#endif
  }
}

void sleepInterruptable(unsigned long cycleBegin, unsigned long periodMs) {
  unsigned long spentMs = millis() - cycleBegin;
  int dc = (spentMs * 100) / periodMs;
  log(CLASS_MAIN, Info, "D.C.:%d%%", dc);
  if (dc > DUTY_CYCLE_THRESHOLD_PERC) {
    log(CLASS_MAIN, Warn, "Cycle: %lums", spentMs);
  }
  log(CLASS_MAIN, Info, "L.Sleep(%lums)...", periodMs);
  while (spentMs < periodMs) {
    if (haveToInterrupt()) {
      break;
    }
    unsigned long fragToSleepMs = MINIM(periodMs - spentMs, FRAG_TO_SLEEP_MS_MAX);
    wifi_set_sleep_type(LIGHT_SLEEP_T);
    delay(fragToSleepMs);
    spentMs = millis() - cycleBegin;
  }
}
