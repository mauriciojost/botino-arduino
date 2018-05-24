
#ifndef UNIT_TEST
#include <Arduino.h>
#include <Module.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>
#include "EspSaveCrash.h"
#include <Pinout.h>
#include "Images.h"
#include "main4ino/Misc.h"
#include "RemoteDebug.h"

#define CLASS_MAIN "MA"

#ifndef WIFI_SSID_INIT
#error "Must provide WIFI_SSID_INIT"
#endif

#ifndef WIFI_PASSWORD_INIT
#error "Must provide WIFI_PASSWORD_INIT"
#endif

extern "C" {
#include "user_interface.h"
}

enum ButtonPressed { NoButton = 0, ButtonSetWasPressed, ButtonModeWasPressed };

#define DO_NOT_CLEAR_FIRST false
#define DELAY_MS_SPI 3
#define FRAG_TO_SLEEP_MS_MAX 500


#ifndef SERVO_ARM_STEPS
#define SERVO_ARM_STEPS 50
#endif // SERVO_ARM_STEPS

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

#define SERVO0_STEP_DEGREES (SERVO0_RANGE_DEGREES / MAX_SERVO_STEPS)
#define SERVO1_STEP_DEGREES (SERVO1_RANGE_DEGREES / MAX_SERVO_STEPS)

#ifndef DWEET_IO_API_TOKEN
#error "Must provide DWEET_IO_API_TOKEN"
#endif

#define WAIT_BEFORE_HTTP_MS 1500

#define SERVO_INVERT_POS(p, f) (((f) ? (180 - (p)) : (p)))

Module m;
Servo servoLeft;
Servo servoRight;
Adafruit_SSD1306 lcd(-1);
volatile unsigned char ints = 0;
HTTPClient httpClient;
RemoteDebug RDebug;

char iii[16] = {
						 0b00000000, 0b00000000,
						 0b00000100, 0b00100000,
						 0b00000000, 0b00000000,
						 0b00000000, 0b00000000,
						 0b00000100, 0b00100000,
						 0b00000111, 0b11100000,
						 0b00000000, 0b00000000,
						 0b00000000, 0b00000000
						 };

/******************/
/***  CALLBACKS ***/
/******************/

ICACHE_RAM_ATTR
void buttonPressed() {
  ints++;
  digitalWrite(LEDW_PIN, LOW);
}

void lcdClear(int line) {
  lcd.fillRect(0, line * 8, 128, 8, BLACK);
}

void lcdPrintLogLine(const char *logStr, int line) {
  bool lcdDebugEnabled = m.getSettings()->getLcdDebug();
  if (!lcdDebugEnabled) {
    return;
  }
  lcd.setTextWrap(false);

  lcdClear(line);
  lcd.setTextSize(1);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, line * 8);
  lcd.println(logStr);
  lcd.display();
  delay(DELAY_MS_SPI);
}

void messageOnLcd(int line, const char *str, int size) {
  lcd.clearDisplay();
  lcd.setTextWrap(true);
  lcd.setTextSize(size);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, line * 2 * 8);
  lcd.println(str);
  lcd.display();
  delay(DELAY_MS_SPI);
}

void imgToLcd(char img[]) {
  lcd.clearDisplay();
	for (char yi=0; yi < 8; yi++) {
    for (char xi=0; xi < 2; xi++) {
    	char imgbyte = img[yi * 2 + xi];
    	for (char b = 0; b < 8; b++) {
    		char color = (imgbyte << b) & 0b10000000;
        int16_t xl = (int16_t)xi * 64 + (int16_t)b * 8;
        int16_t yl = (int16_t)yi * 8;
        uint16_t cl = color==0?BLACK:WHITE;
        lcd.fillRect(xl, yl, 8, 8, cl);
        //log(CLASS_MAIN, Info, "LCD xl=%d/yl=%d->color=%d", xl, yl, color);
    	}
    }
	}
  lcd.display();
  delay(DELAY_MS_SPI);
}

void logLine(const char *str) {
  static int i = 0;
  if (i == 0) {
    lcd.fillRect(0, 0, 128, 64, BLACK);
  }
  lcdPrintLogLine(str, i);
  i = (i + 1) % 8;
  Serial.println(str);
  RDebug.printf("%s\n", str);
}

void white() {
  lcd.clearDisplay();
  lcd.invertDisplay(true);
  lcd.display();
  delay(DELAY_MS_SPI);
}

void black() {
  lcd.clearDisplay();
  lcd.invertDisplay(false);
  lcd.display();
  delay(DELAY_MS_SPI);
}
void beSmily() {
  lcd.clearDisplay();
  lcd.drawBitmap(0, 0, happy, 128, 64, WHITE);
  lcd.display();
  delay(DELAY_MS_SPI);
}
void beSad() {
  lcd.clearDisplay();
  lcd.drawBitmap(0, 0, sad, 128, 64, WHITE);
  lcd.display();
  delay(DELAY_MS_SPI);
}

void beNormal() {
  lcd.clearDisplay();
  lcd.drawBitmap(0, 0, normal, 128, 64, WHITE);
  lcd.display();
  delay(DELAY_MS_SPI);
}

void beSleepy() {
  lcd.clearDisplay();
  lcd.drawBitmap(0, 0, sleepy, 128, 64, WHITE);
  lcd.display();
  delay(DELAY_MS_SPI);
}

void arms(int left, int right) {
  static int lastPosL = -1;
  static int lastPosR = -1;

  log(CLASS_MAIN, Debug, "Arms move: %d %d", left, right);
  int targetPosL = SERVO_INVERT_POS(((POSIT(left) % MAX_SERVO_STEPS) * SERVO0_STEP_DEGREES) + SERVO0_BASE_DEGREES, SERVO0_INVERTED);
  int targetPosR = SERVO_INVERT_POS(((POSIT(right) % MAX_SERVO_STEPS) * SERVO1_STEP_DEGREES) + SERVO1_BASE_DEGREES, SERVO1_INVERTED);
  servoLeft.attach(SERVO0_PIN);
  servoRight.attach(SERVO1_PIN);

  // leave as target if first time
  lastPosL = (lastPosL == -1 ? targetPosL : lastPosL);
  lastPosR = (lastPosR == -1 ? targetPosR : lastPosR);

  log(CLASS_MAIN, Info, "Servo left %d->%d", lastPosL, targetPosL);
  log(CLASS_MAIN, Info, "Servo right %d->%d", lastPosR, targetPosR);
  for (int i = 1; i <= SERVO_ARM_STEPS; i++) {
    float factor = ((float)i) / SERVO_ARM_STEPS;
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

bool initWifi(const char *ssid, const char *pass) {
  log(CLASS_MAIN, Info, "Connecting to %s ...", ssid);

  wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    log(CLASS_MAIN, Debug, "Already connected (%s), skipping", WiFi.localIP().toString().c_str());
    return true; // connected
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF); // to be removed after SDK update to 1.5.4
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  int attemptsLeft = 10;
  while (true) {
    status = WiFi.status();
    log(CLASS_MAIN, Info, " attempts %d", attemptsLeft);
    attemptsLeft--;
    if (status == WL_CONNECTED) {
      log(CLASS_MAIN, Info, "IP: %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
    if (attemptsLeft <= 0) {
      log(CLASS_MAIN, Warn, "Connection failed %d", status);
      return false; // not connected
    }
    delay(1500);
  }
}

bool initWifiInit() {
  log(CLASS_PROPSYNC, Info, "Init wifi init %s", WIFI_SSID_INIT);
  return initWifi(WIFI_SSID_INIT, WIFI_PASSWORD_INIT);
}

bool initWifiSteady() {
  const char *wifiSsid = m.getSetupSync()->getSsid();
  const char *wifiPass = m.getSetupSync()->getPass();
  log(CLASS_PROPSYNC, Info, "Init wifi steady %s", wifiSsid);
  return initWifi(wifiSsid, wifiPass);
}

int httpGet(const char *url, ParamStream *response) {
  httpClient.begin(url);
  httpClient.addHeader("Content-Type", "application/json");
  httpClient.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);

  int errorCode = httpClient.GET();
  log(CLASS_MAIN, Info, "HTTP GET: %s %d", url, errorCode);

  if (errorCode > 0) {
    response->flush();
    httpClient.writeToStream(response);
  } else {
    log(CLASS_MAIN, Error, "! %s", httpClient.errorToString(errorCode).c_str());
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
  log(CLASS_MAIN, Info, "HTTP POST: %s %d", url, errorCode);

  if (errorCode > 0) {
    response->flush();
    httpClient.writeToStream(response);
  } else {
    log(CLASS_MAIN, Error, "! %s", httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
}

void ios(char led, bool v) {
  log(CLASS_MAIN, Debug, "Led '%c' -> %d", led, (int)v);
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

/*****************/
/***** SETUP *****/
/*****************/

void setup() {
  // Let HW startup
  delay(3 * 1000);

  // Initialize the serial port
  Serial.begin(115200);

  // Initialize the LCD
  lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  lcd.dim(true);
  delay(DELAY_MS_SPI);

  // Intialize the remote logging framework
  RDebug.begin("ESP");

  // Intialize the logging framework
  setupLog(logLine);

  log(CLASS_MAIN, Debug, "Setup pins");
  pinMode(LEDR_PIN, OUTPUT);
  pinMode(LEDW_PIN, OUTPUT);
  pinMode(LEDY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(SERVO0_PIN, OUTPUT);
  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(BUTTON0_PIN, INPUT);

  log(CLASS_MAIN, Debug, "Setup module");
  m.setup();

  m.getBody()->setSmilyFace(beSmily);
  m.getBody()->setSadFace(beSad);
  m.getBody()->setNormalFace(beNormal);
  m.getBody()->setSleepyFace(beSleepy);
  m.getBody()->setBlackFace(black);
  m.getBody()->setWhiteFace(white);
  m.getBody()->setArms(arms);
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

  log(CLASS_MAIN, Debug, "Setup interrupts");
  attachInterrupt(digitalPinToInterrupt(BUTTON0_PIN), buttonPressed, RISING);

  log(CLASS_MAIN, Debug, "Init HW test routine"); delay(2000);
  ios('r', false);
  ios('y', false);
  ios('w', false);
  ios('f', false);

  log(CLASS_MAIN, Debug, "...Face weird");
  imgToLcd(iii);; delay(5000);

  log(CLASS_MAIN, Debug, "...Face normal"); delay(2000);
  beNormal();
  log(CLASS_MAIN, Debug, "...Face smily"); delay(2000);
  beSmily();
  log(CLASS_MAIN, Debug, "...Arms down"); delay(2000);
  arms(0, 0);
  log(CLASS_MAIN, Debug, "...Right arm up"); delay(2000);
  arms(0, 9);
  log(CLASS_MAIN, Debug, "...Left arm up"); delay(2000);
  arms(9, 9);
  log(CLASS_MAIN, Debug, "...Arms down"); delay(2000);
  arms(0, 0);
  log(CLASS_MAIN, Debug, "...Red led on"); delay(2000);
  ios('r', true);
  log(CLASS_MAIN, Debug, "...Red led off"); delay(2000);
  ios('r', false);
  log(CLASS_MAIN, Debug, "...Yellow led on"); delay(2000);
  ios('y', true);
  log(CLASS_MAIN, Debug, "...Yellow led off"); delay(2000);
  ios('y', false);
  log(CLASS_MAIN, Debug, "...White led on"); delay(2000);
  ios('w', true);
  log(CLASS_MAIN, Debug, "...White led off"); delay(2000);
  ios('w', false);
  log(CLASS_MAIN, Debug, "...Fan on"); delay(2000);
  ios('f', true);
  log(CLASS_MAIN, Debug, "...Fan off"); delay(2000);
  ios('f', false);

}

void lightSleep(unsigned long delayMs) {
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  delay(delayMs);
}

void handleDebug() {
  Settings* s = m.getSettings();

	// Handle stack-traces stored in memory
  if (s->getClear()) {
    log(CLASS_MAIN, Debug, "Clearing stack-traces");
    SaveCrash.clear();
  } else {
  	if (SaveCrash.count() > 0) {
      log(CLASS_MAIN, Warn, "Found stack-traces (!!!)");
      SaveCrash.print();
  	} else {
      log(CLASS_MAIN, Debug, "No stack-traces");
  	}
  }

  // Handle telnet log server
  RDebug.handle();

  // Handle log level as per settings
  setLogLevel((char)(s->getLogLevel() % 4));

}

void reactButton() {
  if (ints > 0) {
    m.getSettings()->incrButtonPressed((int)ints);
  	m.getBody()->performMove(0);
    digitalWrite(LEDW_PIN, HIGH);
    ints = 0;
  }
}

void loop() {

  unsigned long t1 = millis();

  m.loop(false, false, true);

  handleDebug();

  unsigned long periodMs = m.getSettings()->getPeriodSeconds() * 1000;
  unsigned long spentMs = 0;

  log(CLASS_MAIN, Info, "Li-sleep ( %lu ms)...", periodMs);
  while(spentMs < periodMs) {

    reactButton();
  	unsigned long fragToSleepMs = MINIM(periodMs - spentMs, FRAG_TO_SLEEP_MS_MAX);
    lightSleep(fragToSleepMs);

    spentMs = millis() - t1;

  }
}

#endif // UNIT_TEST
