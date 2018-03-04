#ifndef UNIT_TEST
#include <Main.h>

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

volatile char nroInterruptsQueued = 0; // counter to keep track of amount of timing
                                       // interrupts queued

enum ButtonPressed {
  NoButton = 0,
  ButtonSetWasPressed,
  ButtonModeWasPressed
};

Module m;

/*****************/
/** INTERRUPTS ***/
/*****************/

ICACHE_RAM_ATTR void timingInterrupt(void) {
  nroInterruptsQueued++;
}


/******************/
/***  CALLBACKS ***/
/******************/

void displayOnLogs(const char *str1, const char *str2) {
  log(CLASS, Info, str1);
  log(CLASS, Info, str2);

  Buffer<LCD_LINE_LENGTH> b;
  b.load(str1);
  m.getLcd()->setProp(LcdConfigLineUpAState, SetValue, &b, NULL);
  b.load(str2);
  m.getLcd()->setProp(LcdConfigLineDownAState, SetValue, &b, NULL);
}

/*****************/
/***** SETUP *****/
/*****************/

void setupPins() {
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  //pinMode(BUZZER0_PIN, OUTPUT); // will break serial communication
  log(CLASS, Info, "PINS READY");
}

//void setupInterrupts() {
//  timer1_disable();
//  timer1_isr_init();
//  timer1_attachInterrupt(timingInterrupt);
//  timer1_enable(TIM_DIV265, TIM_EDGE, TIM_LOOP);
//  timer1_write(TICKS_PERIOD_TIMER1);
//  log(CLASS, Info, "INT READY");
//}

void setup() {
  setupLog();
  delay(1000);
  log(CLASS, Info, "LOG READY");

  setupPins();
  m.setup();
  m.setStdoutWriteFunction(displayOnLogs);
  m.setDigitalWriteFunction(digitalWrite);
  //setupInterrupts();
}

ButtonPressed readButtons() {
  if (readAvailable() > 0) {
    int b = readByte();
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

void doDelays();
void initWifi();

const char* ssid = "ssid";
const char* password = "pass";

void loop() {

  initWifi();

  Serial.println("Light sleep & delays:");
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  doDelays();

  Serial.println("Run module:");
  ButtonPressed button = readButtons();
  log(CLASS, Info, "INT");
  m.loop(button == ButtonModeWasPressed, button == ButtonSetWasPressed, true);

  Serial.println("None sleep & delays:");
  wifi_set_sleep_type(NONE_SLEEP_T);
  doDelays();

  WiFi.disconnect();
  Serial.print("WiFi disconnected, IP address: "); Serial.println(WiFi.localIP());
  Serial.println("Light sleep & delays:");
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  doDelays();

}

void doDelays() {
  Serial.println("Yield for 1 sec");
  long endMs = millis() + 1000;
  while (millis() < endMs) {
     yield();
  }

  Serial.println("Delay for 1 sec");
  delay(1000);
}

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while ((WiFi.status() != WL_CONNECTED)) {
     delay(500);
     Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

#endif // UNIT_TEST
