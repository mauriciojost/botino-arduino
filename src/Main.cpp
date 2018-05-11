
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
#define CLEAR_FIRST true
#define DELAY_MS_SPI 2
#define NRO_LEDS 3

#ifndef SERVO_ARM_STEPS
#define SERVO_ARM_STEPS 100
#endif // SERVO_ARM_STEPS

#ifndef ARM_UP_SERVO_POS
#define ARM_UP_SERVO_POS 140
#endif // ARM_UP_SERVO_POS

#ifndef ARM_MIDDLE_SERVO_POS
#define ARM_MIDDLE_SERVO_POS 90
#endif // ARM_MIDDLE_SERVO_POS

#ifndef ARM_DOWN_SERVO_POS
#define ARM_DOWN_SERVO_POS 50
#endif // ARM_DOWN_SERVO_POS

#ifndef SERVO0_INVERTED
#define SERVO0_INVERTED true
#endif // SERVO0_INVERTED

#ifndef SERVO1_INVERTED
#define SERVO1_INVERTED false
#endif // SERVO1_INVERTED

#ifndef DWEET_IO_API_TOKEN
#error "Must provide DWEET_IO_API_TOKEN"
#endif

#define WAIT_BEFORE_HTTP_MS 1500

#define INVERT(p, f) ((f ? 180 - p : p))

Module m;
Servo servoLeft;
Servo servoRight;
Adafruit_SSD1306 lcd(-1);
volatile unsigned char ints = 0;
HTTPClient httpClient;
RemoteDebug RDebug;

/******************/
/***  CALLBACKS ***/
/******************/

void buttonPressed() {
  ints++;
}

void lcdClear() {
  lcd.clearDisplay();
}

void lcdClear(int line) {
  lcd.fillRect(0, line * 8, 128, 8, BLACK);
}

void lcdPrintLine(const char *str, int line, bool clearFirst) {
  static bool cleared = false;
  lcd.setTextWrap(false);

  bool disableLcd = m.getSettings()->getDisableLcd();
  if (disableLcd) {
    if (!cleared) {
      lcdClear();
      lcd.display();
      delay(DELAY_MS_SPI);
      cleared = true;
    }
    return;
  }
  if (clearFirst) {
    lcdClear(line);
  }
  lcd.setTextSize(1);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, line * 8);
  lcd.println(str);
  lcd.display();
  delay(DELAY_MS_SPI);
  cleared = false;
}

void messageOnLcd(int line, const char *str) {
  lcd.clearDisplay();
  lcd.setTextWrap(true);
  lcd.setTextSize(2);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, line * 2 * 8);
  lcd.println(str);
  lcd.display();
  delay(DELAY_MS_SPI);
  delay(2000);
}

void logLine(const char *str) {
  static int i = 0;
  if (i == 0) {
    lcd.fillRect(0, 0, 128, 64, BLACK);
  }
  i = (i + 1) % 8;
  lcdPrintLine(str, i, CLEAR_FIRST);
  Serial.println(str);
  RDebug.printf("%s\n", str);
}

void beClear() {
  lcd.clearDisplay();
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

int getServoPosition(ArmState a, bool inverted) {
  switch (a) {
    case ArmUp:
      return INVERT(ARM_UP_SERVO_POS, inverted);
    case ArmMiddle:
      return INVERT(ARM_MIDDLE_SERVO_POS, inverted);
    case ArmDown:
      return INVERT(ARM_DOWN_SERVO_POS, inverted);
    default:
      return 0;
  }
}

void arms(ArmState left, ArmState right) {
  static int lastPosL = 0;
  static int lastPosR = 0;
  log(CLASS_MAIN, Debug, "Arms move");
  int targetPosL = getServoPosition(left, SERVO0_INVERTED);
  int targetPosR = getServoPosition(right, SERVO1_INVERTED);
  servoLeft.attach(SERVO0_PIN);
  servoRight.attach(SERVO1_PIN);
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
  log(CLASS_PROPSYNC, Debug, "HTTP GET");
  httpClient.begin(url);
  httpClient.addHeader("Content-Type", "application/json");
  httpClient.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);

  int errorCode = httpClient.GET();
  log(CLASS_PROPSYNC, Info, "HTTP GET: %s %d", url, errorCode);

  if (errorCode > 0) {
    response->flush();
    httpClient.writeToStream(response);
  } else {
    log(CLASS_PROPSYNC, Error, "! %s", httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
}

int httpPost(const char *url, const char *body, ParamStream *response) {
  log(CLASS_PROPSYNC, Debug, "HTTP POST");
  httpClient.begin(url);
  httpClient.addHeader("Content-Type", "application/json");
  httpClient.addHeader("X-Auth-Token", DWEET_IO_API_TOKEN);

  int errorCode = httpClient.POST(body);
  log(CLASS_PROPSYNC, Info, "HTTP POST: %s %d", url, errorCode);

  if (errorCode > 0) {
    response->flush();
    httpClient.writeToStream(response);
  } else {
    log(CLASS_PROPSYNC, Error, "! %s", httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
}

void led(unsigned char led, unsigned char v) {
  unsigned char l = led % NRO_LEDS;
  log(CLASS_MAIN, Debug, "Led %d -> %d", (int)l, (int)v);
  switch (l) {
    case 0:
      digitalWrite(LED0_PIN, v);
      break;
    case 1:
      digitalWrite(LED1_PIN, v);
      break;
    case 2:
      digitalWrite(LED2_PIN, v);
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

  // Print exception raised during previous run (if any)
  SaveCrash.print();

  // Initialize the LCD
  lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  lcd.dim(true);
  delay(DELAY_MS_SPI);

  // Intialize the remote logging framework
  RDebug.begin("ESP");

  // Intialize the logging framework
  setupLog(logLine);

  log(CLASS_MAIN, Debug, "Setup pins");
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(SERVO0_PIN, OUTPUT);
  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(BUTTON0_PIN, INPUT);

  log(CLASS_MAIN, Debug, "Setup module");
  m.setup();

  m.getBody()->setSmilyFace(beSmily);
  m.getBody()->setSadFace(beSad);
  m.getBody()->setNormalFace(beNormal);
  m.getBody()->setSleepyFace(beSleepy);
  m.getBody()->setClearFace(beClear);
  m.getBody()->setArms(arms);
  m.getBody()->setMessageFunc(messageOnLcd);
  m.getBody()->setLedFunc(led);
  m.getPropSync()->setInitWifi(initWifiSteady);
  m.getPropSync()->setHttpPost(httpPost);
  m.getPropSync()->setHttpGet(httpGet);
  m.getClockSync()->setInitWifi(initWifiSteady);
  m.getClockSync()->setHttpGet(httpGet);
  m.getSetupSync()->setInitWifiSteady(initWifiSteady);
  m.getSetupSync()->setInitWifiInit(initWifiInit);
  m.getSetupSync()->setHttpGet(httpGet);

  log(CLASS_MAIN, Debug, "Setup interrupts");
  attachInterrupt(digitalPinToInterrupt(BUTTON0_PIN), buttonPressed, FALLING);

  log(CLASS_MAIN, Debug, "Init HW test routine");
  digitalWrite(LED0_PIN, HIGH);
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  beNormal();
  arms(ArmDown, ArmDown);

  delay(2000);
  digitalWrite(LED0_PIN, LOW);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  beSmily();
  arms(ArmMiddle, ArmMiddle);

  delay(2000);
  digitalWrite(LED0_PIN, HIGH);
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  beSleepy();
  arms(ArmDown, ArmDown);
}

/**
 * Read buttons from serial port
 * TODO: make evolve to read physical buttons
 */
ButtonPressed readButtons() {
  if (Serial.available() > 0) {
    int b = Serial.read();
    switch (b) {
      case 's':
      case 'S':
        return ButtonSetWasPressed;
      case 'm':
      case 'M':
        return ButtonModeWasPressed;
      default:
        return NoButton;
    }
  }
}

void lightSleep(unsigned long delayMs) {
  log(CLASS_MAIN, Info, "Li-sleep (%lu ms)...", delayMs);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  delay(delayMs);
}

void deepSleep(uint32_t delayUs) {
  // RST to GPIO16
  // Sometimes hangs https://github.com/esp8266/Arduino/issues/2049
  log(CLASS_MAIN, Info, "De-sleep (%lu us)...", delayUs);
  ESP.deepSleep(delayUs);
}

void loop() {

  unsigned long t1 = millis();

  ButtonPressed button = readButtons();
  m.loop(button == ButtonModeWasPressed, button == ButtonSetWasPressed, true);

  if (m.getSettings()->getClear()) {
    log(CLASS_MAIN, Debug, "Clearing save crash");
    SaveCrash.clear();
  }

  m.getSettings()->setButtonPressed((int)ints);

  setLogLevel((char)(m.getSettings()->getLogLevel() % 4));

  RDebug.handle();

  unsigned long t2 = millis();
  unsigned long periodMs = m.getSettings()->getPeriodSeconds() * 1000;
  unsigned long spentPeriodMs = MINIM(POSIT(t2 - t1), periodMs);
  lightSleep(periodMs - spentPeriodMs);

  log(CLASS_MAIN, Info, "Free heap: %ld", ESP.getFreeHeap());
}

#endif // UNIT_TEST
