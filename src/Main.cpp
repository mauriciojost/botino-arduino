
#ifndef UNIT_TEST
#include <Main.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include "EspSaveCrash.h"
#include <Pinout.h>
#include "Images.h"

#define CLASS_MAIN "MA"

#ifndef WIFI_SSID
#error "Must provide WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#error "Must provide WIFI_PASSWORD"
#endif

extern "C" {
#include "user_interface.h"
}

enum ButtonPressed { NoButton = 0, ButtonSetWasPressed, ButtonModeWasPressed };

#define DO_NOT_CLEAR_FIRST false
#define CLEAR_FIRST true
#define DELAY_MS_SPI 2

Module m;
Servo servoLeft;
Servo servoRight;
Adafruit_SSD1306 lcd(-1);
volatile unsigned char ints = 0;

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

/*
 * LCD (128 x 64 pixels, 16 x 8 chars):
 *
 *   BOT X LINE 0
 *   BOT X LINE 1
 *
 *   LCD X LINE 0
 *   LCD X LINE 1
 *
 *   LOGS0
 *   LOGS1
 *   LOGS2
 *   LOGS3
 *
 * */

void botDisplayOnLcd(const char *str1, const char *str2) {
  lcdPrintLine(str1, 0, CLEAR_FIRST);
  lcdPrintLine(str2, 1, CLEAR_FIRST);
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
  int line = i + 4;
  if (i == 0) {
    lcd.fillRect(0, 4 * 8, 128, 4 * 8, BLACK);
  }
  i = (i + 1) % 4;
  lcdPrintLine(str, line, CLEAR_FIRST);
  Serial.println(str);
}

/*****************/
/***** SETUP *****/
/*****************/


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

int smooth(int pin, Servo *servo, int lastPos, int targetPos, int steps) {
  log(CLASS_MAIN, Info, "serv0 %d->%d", lastPos, targetPos);
  if (lastPos != targetPos) {
    servo->attach(pin);
    for (int i = 1; i <= steps; i++) {
      float factor = ((float)i) / steps;
        int v = lastPos + ((targetPos - lastPos) * factor);
        servo->write(v);
      delay(15);
    }
    servo->detach();
  }
  return targetPos;
}

int arm(Servo *servo, ArmState a, int lastPos, int pin) {

#define SERVO_ARM_STEPS 100
#define ARM_UP_SERVO_POS 0
#define ARM_MIDDLE_SERVO_POS 90
#define ARM_DOWN_SERVO_POS 180

  // Right arm ignored for now
  switch (a) {
    case ArmUp:
      return smooth(pin, servo, lastPos, ARM_UP_SERVO_POS, SERVO_ARM_STEPS); // oups, messed up with HW, TODO: FIXME
    case ArmMiddle:
      return smooth(pin, servo, lastPos, ARM_MIDDLE_SERVO_POS, SERVO_ARM_STEPS);
    case ArmDown:
      return smooth(pin, servo, lastPos, ARM_DOWN_SERVO_POS, SERVO_ARM_STEPS);
    default:
    	return lastPos;
  }
}

void arms(ArmState left, ArmState right) {
  static int rightPos = 0;
  static int leftPos = 0;
  leftPos = arm(&servoLeft, left, leftPos, SERVO0_PIN);
  rightPos = arm(&servoRight, right, rightPos, SERVO1_PIN);
}

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

  // Intialize the logging framework
  setupLog(logLine);

  log(CLASS_MAIN, Debug, "Setup pins");
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  //pinMode(SERVO0_PIN, OUTPUT);
  //pinMode(SERVO1_PIN, OUTPUT);
  //pinMode(BUTTON0_PIN, INPUT);

  log(CLASS_MAIN, Debug, "Setup module");
  m.setup();
  m.getBot()->setStdoutFunction(botDisplayOnLcd);
  m.setDigitalWriteFunction(digitalWrite);

  m.getBody()->setSmilyFace(beSmily);
  m.getBody()->setSadFace(beSad);
  m.getBody()->setNormalFace(beNormal);
  m.getBody()->setSleepyFace(beSleepy);
  m.getBody()->setArms(arms);
  m.getBody()->setMessageFunc(messageOnLcd);

  log(CLASS_MAIN, Debug, "Setup interrupts");
  attachInterrupt(digitalPinToInterrupt(BUTTON0_PIN), buttonPressed, FALLING);
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

wl_status_t initWifi() {
  int attemptsLeft = 10;
  log(CLASS_MAIN, Info, "Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (true) {
    wl_status_t status = WiFi.status();
    log(CLASS_MAIN, Info, " attempts %d", attemptsLeft);
    attemptsLeft--;
    if (status == WL_CONNECTED) {
      log(CLASS_MAIN, Info, "IP: %s", WiFi.localIP().toString().c_str());
      return status;
    }
    if (attemptsLeft <= 0) {
      log(CLASS_MAIN, Warn, "Connection failed %d", status);
      return status;
    }
    delay(1500);
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

  wl_status_t wifiStatus = initWifi();

  if (wifiStatus == WL_CONNECTED) {
    ButtonPressed button = readButtons();
    m.loop(button == ButtonModeWasPressed, button == ButtonSetWasPressed, true);
  }

  if (m.getSettings()->getClear()) {
    SaveCrash.clear();
  }

  m.getSettings()->setButtonPressed((int)ints);

  setLogLevel((char)(m.getSettings()->getLogLevel() % 4));

  unsigned long t2 = millis();
  unsigned long periodMs = m.getSettings()->getPeriodSeconds() * 1000;
  unsigned long spentPeriodMs = MINIM(POSIT(t2 - t1), periodMs);
  lightSleep(periodMs - spentPeriodMs);
}

#endif // UNIT_TEST
