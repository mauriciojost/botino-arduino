#ifndef UNIT_TEST
#include <Main.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);

#define CLASS "Main"
#define TICKS_PERIOD_TIMER1 300000
#define SLEEP_DELAY_US 1000 * 1000 * 5
#define FPM_SLEEP_MAX_TIME 0xFFFFFFF
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

Module m;

/******************/
/***  CALLBACKS ***/
/******************/

void logs(const char *str) {
  static int i = 0;
  i = (i + 1) % 8;
  if (i == 0) {
    display.clearDisplay();
  }
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,i * 8);
  display.println("                ");
  display.setCursor(0,i * 8);
  display.println(str);
  display.display();
}


void displayOnLogs(const char *str1, const char *str2) {
  Buffer<LCD_LINE_LENGTH> b;
  b.load(str1);
  m.getLcd()->setProp(LcdConfigChan0Line0, SetValue, &b, NULL);
  b.load(str2);
  m.getLcd()->setProp(LcdConfigChan0Line1, SetValue, &b, NULL);

  logs(str1);
  logs(str2);
}

/*****************/
/***** SETUP *****/
/*****************/

void setupPins() {
  //pinMode(LED0_PIN, OUTPUT); // will break deep sleep mode
  //pinMode(LED1_PIN, OUTPUT); // will break deep sleep mode
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  //pinMode(BUZZER0_PIN, OUTPUT); // will break serial communication
  log(CLASS, Info, "PINS READY");
}

void setup() {
  delay(3*000);
  Serial.begin(115200);
  setupLog(logs);
  log(CLASS, Info, "LOG READY");

  setupPins();
  m.setup();
  m.setStdoutWriteFunction(displayOnLogs);
  m.setDigitalWriteFunction(digitalWrite);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the buffer.
  display.clearDisplay();
  display.display();
}

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
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while ((WiFi.status() != WL_CONNECTED)) {
     delay(400);
     logs(".");
  }
  logs("WiFi connected");
}


void loop() {

  logs("Init WIFI...");
  initWifi();

  Serial.println("None sleep...");
  wifi_set_sleep_type(NONE_SLEEP_T);
  delay(6000);

  logs("Run module...");
  ButtonPressed button = readButtons();
  log(CLASS, Info, "INT");
  m.loop(button == ButtonModeWasPressed, button == ButtonSetWasPressed, true);

  logs("Light sleep...");
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  delay(6000);

  WiFi.disconnect();
  Serial.println("Light sleep (disc)...");
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  delay(6000);

  display.dim(true);
  logs("Deep sleep...");
  ESP.deepSleep(10e6);
  delay(60000);
  display.dim(false);

}


#endif // UNIT_TEST
