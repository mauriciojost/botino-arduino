#ifndef UNIT_TEST
#include <Main.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);

#define CLASS "MA"
#ifndef WIFI_SSID
#error "Must provide WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#error "Must provide WIFI_PASSWORD"
#endif


extern "C" {
#include "user_interface.h"
}

enum ButtonPressed {
  NoButton = 0,
  ButtonSetWasPressed,
  ButtonModeWasPressed
};

#define DO_NOT_CLEAR_FIRST false
#define CLEAR_FIRST true

Module m;

/******************/
/***  CALLBACKS ***/
/******************/

void lcdPrintLine(const char *str, int y, bool clearFirst, int color) {
  if (clearFirst) {
    display.fillRect(0, y * 8, 128, 8, BLACK);
  }
  display.setTextSize(1);
  display.setTextColor(color);
  display.setCursor(0, y * 8);
  display.println(str);
  display.display();
}

/*
 *
 * CHANNEL X LINE 0 |
 * CHANNEL X LINE 1 |
 * LOGS0
 * LOGS1
 * LOGS2
 * LOGS3
 * LOGS4
 * LOGS5
 *
 * */

void logLine(const char *str) {
  static int i = 0;
  if (i == 0) {
    display.fillRect(0, 16, 128, 8 * 6, WHITE);
  }
  int y = i + 2;
  lcdPrintLine(str, y, DO_NOT_CLEAR_FIRST, BLACK);
  i = (i + 1) % 6;
}


void displayOnLcd(const char *str1, const char *str2) {
  // Update properties of the logical display actor (channel 0)
  Buffer<LCD_LINE_LENGTH> b1;
  Buffer<LCD_LINE_LENGTH> b2;

  b1.load(str1);
  b2.load(str2);

  m.getLcd()->setProp(LcdConfigChan0Line0, SetValue, &b1, NULL);
  m.getLcd()->setProp(LcdConfigChan0Line1, SetValue, &b2, NULL);

  // Update the physical display (depending on channel to show)
  if (m.getLcd()->getChannel() == 0) {
    lcdPrintLine(str1, 0, CLEAR_FIRST, WHITE);
    lcdPrintLine(str2, 1, CLEAR_FIRST, WHITE);
  } else {
	m.getLcd()->setProp(LcdConfigChan1Line0, DoNotSet, &b1, &b1);
	m.getLcd()->setProp(LcdConfigChan1Line1, DoNotSet, &b2, &b2);
    lcdPrintLine(b1.getBuffer(), 0, CLEAR_FIRST, WHITE);
    lcdPrintLine(b2.getBuffer(), 1, CLEAR_FIRST, WHITE);
  }
}

/*****************/
/***** SETUP *****/
/*****************/

void setupPins() {
  log(CLASS, Debug, "Setup pins");
  //pinMode(LED0_PIN, OUTPUT);
  //pinMode(LED1_PIN, OUTPUT); // will break deep sleep mode
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  //pinMode(BUZZER0_PIN, OUTPUT);
}

void setup() {
  // Let HW startup
  delay(3*1000);

  // Initialize the LCD
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  // Intialize the logging framework
  setupLog(logLine);

  // Initialize the serial port
  Serial.begin(115200);

  // Initialize pins
  setupPins();

  // Intialize the module
  m.setup();
  m.setStdoutWriteFunction(displayOnLcd);
  m.setDigitalWriteFunction(digitalWrite);

}

/**
 * Read buttons from serial port
 * TODO: make evolve to read physical buttons
 */
ButtonPressed readButtons() {
  if (Serial.available() > 0) {
    int b = Serial.read();
    switch(b) {
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

void initWifi() {
  log(CLASS, Info, "Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while ((WiFi.status() != WL_CONNECTED)) {
    log(CLASS, Debug, "Connecting...");
    delay(400);
  }
  log(CLASS, Info, "IP: %s", WiFi.localIP().toString().c_str());
}

void lightSleep(unsigned long delayMs) {
  log(CLASS, Debug, "Dim");
  display.dim(true);
  log(CLASS, Info, "Light sleep...");
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  delay(delayMs);
  log(CLASS, Debug, "Undim");
  display.dim(false);
}

void deepSleep(uint32_t delayUs) {
  log(CLASS, Debug, "Dim");
  display.dim(true);
  // RST to GPIO16
  // Sometimes hangs https://github.com/esp8266/Arduino/issues/2049
  log(CLASS, Info, "Deep sleep...");
  ESP.deepSleep(delayUs);
  log(CLASS, Debug, "Undim");
  display.dim(false);
}


void loop() {

  initWifi();

  ButtonPressed button = readButtons();
  m.loop(button == ButtonModeWasPressed, button == ButtonSetWasPressed, true);

  lightSleep(10 * 1000);

}

#endif // UNIT_TEST
