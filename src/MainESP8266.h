
#include <Main.h>

#include "EspSaveCrash.h"
#include "RemoteDebug.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <Pinout.h>
#include <SPI.h>
#include <Servo.h>
#include <Wire.h>

#define DELAY_MS_SPI 3

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

#define DEV_USER_DELAY_MS 1000

#define LOG_LINE 7

#ifndef WIFI_SSID_INIT
#error "Must provide WIFI_SSID_INIT"
#define WIFI_SSID_INIT ""
#endif

#ifndef WIFI_PASSWORD_INIT
#error "Must provide WIFI_PASSWORD_INIT"
#define WIFI_PASSWORD_INIT ""
#endif

#define ONLY_SHOW_MSG true
#define SHOW_MSG_AND_REACT false

extern "C" {
#include "user_interface.h"
}

#define HTTP_TIMEOUT_MS 8000

volatile unsigned char buttonInterrupts = 0;

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

#define LED_INT_PIN LEDY_PIN // led showing interruptions
#define LED_INT_ON digitalWrite(LED_INT_PIN, LOW)
#define LED_INT_OFF digitalWrite(LED_INT_PIN, HIGH)

#define LED_ALIVE_PIN LEDR_PIN // led showing device alive
#define LED_ALIVE_ON digitalWrite(LED_ALIVE_PIN, LOW);
#define LED_ALIVE_OFF digitalWrite(LED_ALIVE_PIN, HIGH);

void bitmapToLcd(uint8_t bitmap[]);
void reactCommandCustom();
void hwTest();
void buttonPressed();
bool reactToButtonHeld(int cycles, bool onlyMsg);

////////////////////////////////////////
// Functions requested for architecture
////////////////////////////////////////

void logLine(const char *str) {
  // serial print
  Serial.print(str);
  // telnet print
  if (Telnet.isActive()) {
    Telnet.printf("%s", str);
  }
  // lcd print
  if (m.getSettings()->getDebug()) {
    lcd.setTextWrap(false);
    lcd.fillRect(0, LOG_LINE * 8, 128, 8, BLACK);
    lcd.setTextSize(1);
    lcd.setTextColor(WHITE);
    lcd.setCursor(0, LOG_LINE * 8);
    lcd.print(str);
    lcd.display();
    delay(DELAY_MS_SPI);
  }
}

bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries) {
  wl_status_t status;
  log(CLASS_MAIN, Info, "To '%s'...", ssid);

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
    delay(WIFI_DELAY_MS);
    WiFi.mode(WIFI_OFF); // to be removed after SDK update to 1.5.4
    delay(WIFI_DELAY_MS);
  }

  WiFi.mode(WIFI_STA);
  delay(WIFI_DELAY_MS);
  WiFi.begin(ssid, pass);

  int attemptsLeft = retries;
  while (true) {
    delay(WIFI_DELAY_MS);
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

// TODO: add https support, which requires fingerprint of server that can be obtained as follows:
//  openssl s_client -connect dweet.io:443 < /dev/null 2>/dev/null | openssl x509 -fingerprint -noout -in /dev/stdin
int httpGet(const char *url, ParamStream *response, Table *headers) {
  httpClient.begin(url);
  int i = 0;
  while ((i = headers->next(i)) != -1) {
    httpClient.addHeader(headers->getKey(i), headers->getValue(i));
    i++;
  }
  log(CLASS_MAIN, Debug, "> GET:..%s", tailStr(url, URL_PRINT_MAX_LENGTH));
  int errorCode = httpClient.GET();
  log(CLASS_MAIN, Debug, "> GET:%d", errorCode);

  if (errorCode == HTTP_OK || errorCode == HTTP_NO_CONTENT) {
    if (response != NULL) {
      httpClient.writeToStream(response);
    }
  } else {
    int e = httpClient.writeToStream(&Serial);
    log(CLASS_MAIN, Error, "> GET(%d):%d %s", e, errorCode, httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
}

int httpPost(const char *url, const char *body, ParamStream *response, Table *headers) {
  httpClient.begin(url);
  int i = 0;
  while ((i = headers->next(i)) != -1) {
    httpClient.addHeader(headers->getKey(i), headers->getValue(i));
    i++;
  }

  log(CLASS_MAIN, Debug, "> POST:..%s", tailStr(url, URL_PRINT_MAX_LENGTH));
  log(CLASS_MAIN, Debug, "> POST:'%s'", body);
  int errorCode = httpClient.POST(body);
  log(CLASS_MAIN, Debug, "> POST:%d", errorCode);

  if (errorCode == HTTP_OK || errorCode == HTTP_CREATED) {
    if (response != NULL) {
      httpClient.writeToStream(response);
    }
  } else {
    int e = httpClient.writeToStream(&Serial);
    log(CLASS_MAIN, Error, "> POST(%d):%d %s", e, errorCode, httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
}

void messageFunc(int x, int y, int color, bool wrap, bool clear, int size, const char *str) {
  int l = strlen(str);
  if (clear) {
    lcd.clearDisplay();
  }
  lcd.setTextWrap(wrap);
  lcd.setTextSize(size);
  lcd.setTextColor(!color);
  int i;
  lcd.setCursor(x, y);
  for (i = 0; i < l; i++) {
  	lcd.print("\x07");
  }
  lcd.setCursor(x, y);
  for (i = 0; i < l; i++) {
  	lcd.print("\x08");
  }
  lcd.setTextColor(color);
  lcd.setCursor(x, y);
  lcd.print(str);
  lcd.display();
  delay(DELAY_MS_SPI);
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

void clearDevice() {
  SaveCrash.clear();
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "Img '%c'", img);
  switch (img) {
    case '_': // dim
      log(CLASS_MAIN, Debug, "Dim face");
      lcd.dim(true);
      break;
    case '-': // bright
      log(CLASS_MAIN, Debug, "Bright face");
      lcd.dim(false);
      break;
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

// TODO: buggy (what happens on overrun?), and can be simplified using the clock and time_t
void sleepInterruptable(unsigned long cycleBegin, unsigned long periodMs) {
  unsigned long spentMs = millis() - cycleBegin;
  int dc = (spentMs * 100) / periodMs;

  log(CLASS_MAIN, Debug, "D.C.:%d%%", dc);
  if (dc > DUTY_CYCLE_THRESHOLD_PERC) {
    log(CLASS_MAIN, Debug, "Cycle: %lums", spentMs);
  }
  log(CLASS_MAIN, Debug, "L.Sleep(%lums)...", periodMs);
  while (spentMs < periodMs) {
    if (haveToInterrupt()) {
      break;
    }
    unsigned long fragToSleepMs = MINIM(periodMs - spentMs, FRAG_TO_SLEEP_MS_MAX);
    if (SaveCrash.count() > 0) {
      LED_ALIVE_OFF;
    } else {
      LED_ALIVE_ON;
    }
    delay(fragToSleepMs / 500 * 1);
    if (SaveCrash.count() > 0) {
      LED_ALIVE_ON;
    } else {
      LED_ALIVE_OFF;
    }
    delay(fragToSleepMs / 500 * 499);
    spentMs = millis() - cycleBegin;
  }
}

bool haveToInterrupt() {
  if (Serial.available()) {
    // Handle serial commands
    Buffer cmdBuffer(COMMAND_MAX_LENGTH);
    log(CLASS_MAIN, Info, "Listening...");
    cmdBuffer.clear();
    Serial.readBytesUntil('\n', cmdBuffer.getUnsafeBuffer(), COMMAND_MAX_LENGTH);
    cmdBuffer.replace('\n', '\0');
    cmdBuffer.replace('\r', '\0');
    bool interrupt = m.command(cmdBuffer.getBuffer());
    log(CLASS_MAIN, Debug, "Interrupt: %d", interrupt);
    return interrupt;
  } else if (buttonInterrupts > 0) {
    // Handle button commands
    log(CLASS_MAIN, Debug, "Button command (%d)", buttonInterrupts);
    int holds = 0;
    reactToButtonHeld(holds, ONLY_SHOW_MSG);
    delay(100); // anti-bouncing
    while (digitalRead(BUTTON0_PIN)) {
      holds++;
      log(CLASS_MAIN, Debug, "%d", holds);
      LED_INT_ON;
      reactToButtonHeld(holds, ONLY_SHOW_MSG);
      LED_INT_OFF;
      delay(950);
    }
    bool interruptMe = reactToButtonHeld(holds, SHOW_MSG_AND_REACT);
    LED_INT_OFF;
    buttonInterrupts = 0;

    log(CLASS_MAIN, Debug, "Done");
    return interruptMe;
  } else {
    // Nothing to handle, no reason to interrupt
    return false;
  }
}

void setupArchitecture() {

  // Let HW startup
  delay(2 * 1000);

  // Intialize the logging framework
  Serial.begin(115200);     // Initialize serial port
  Serial.setTimeout(10000); // Timeout for read
  lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(DELAY_MS_SPI); // Initialize LCD
  setupLog(logLine);   // Initialize log callback

  log(CLASS_MAIN, Debug, "Setup timing");
  setExternalMillis(millis);

  log(CLASS_MAIN, Debug, "Setup wdt");
  ESP.wdtEnable(1); // argument not used

  log(CLASS_MAIN, Debug, "Setup wifi");
  WiFi.persistent(false);
  WiFi.hostname(DEVICE_NAME);

  log(CLASS_MAIN, Debug, "Setup http");
  httpClient.setTimeout(HTTP_TIMEOUT_MS);

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
  String helpCli(HELP_COMMAND_CLI);
  Telnet.setHelpProjectsCmds(helpCli);

  hwTest();
}

void runModeArchitecture() {
  Settings *s = m.getSettings();

  // Logs
  log(CLASS_MAIN, Info, "DEV NAME: %s", DEVICE_NAME);
  log(CLASS_MAIN, Info, "IP: %s", WiFi.localIP().toString().c_str());
  log(CLASS_MAIN, Info, "Memory: %lu", ESP.getFreeHeap());
  log(CLASS_MAIN, Info, "Crashes: %d", SaveCrash.count());
  log(CLASS_MAIN, Info, "HTTP size: %d", httpClient.getSize());

  // Handle stack-traces stored in memory
  if (SaveCrash.count() > 0) {
    log(CLASS_MAIN, Warn, "Stack-trcs (!!!)");
    SaveCrash.print();
  }

  // Report interesting information about the device
  Buffer infoBuffer(INFO_BUFFER_LENGTH);
  infoBuffer.fill("crs %d / ver %s / upt %lu h", SaveCrash.count(), STRINGIFY(PROJ_VERSION), (millis() / 1000) / 3600);
  s->setInfo(infoBuffer.getBuffer());

  // Handle log level as per settings
  Serial.setDebugOutput(s->getDebug()); // deep HW logs
}

void configureModeArchitecture() {
  static bool firstTime = true;
  if (firstTime) {
    Telnet.begin("ESP" DEVICE_NAME); // Intialize the remote logging framework
    ArduinoOTA.begin();              // Intialize OTA
  }

  m.getNotifier()->message(0, 1, "telnet %s", WiFi.localIP().toString().c_str());
  Telnet.handle();     // Handle telnet log server and commands
  ArduinoOTA.handle(); // Handle on the air firmware load
}

////////////////////////////////////////
// Architecture specific functions
////////////////////////////////////////

bool reactToButtonHeld(int cycles, bool onlyMsg) {
  switch (cycles) {
    case 0: {
      m.getNotifier()->message(0, 2, "Ack?");
      if (!onlyMsg) {
        m.command("ack");
        m.command("move Z.");
      }
    } break;
    case 1: {
      m.getNotifier()->message(0, 2, "Random routine?");
      if (!onlyMsg) {
        m.command("rnd");
        m.getNotifier()->message(0, 1, "Random routine...");
      }
    } break;
    case 2:
    case 3:
    case 4:
    case 5: {
      int event = cycles - 2;
      const char *evtName = m.getIfttt()->getEventName(event);
      m.getNotifier()->message(0, 2, "Push event %s?", evtName);
      if (!onlyMsg) {
        bool suc = m.getIfttt()->triggerEvent(event);
        if (suc) {
          m.getNotifier()->message(0, 1, "Event %s pushed!", evtName);
        } else {
          m.getNotifier()->message(0, 1, "Failed: event not pushed");
        }
      }
    } break;
    case 6: {
      m.getNotifier()->message(0, 2, "All act?");
      if (!onlyMsg) {
        m.command("actall");
        m.getNotifier()->message(0, 1, "All act one-off");
      }
    } break;
    case 7: {
      m.getNotifier()->message(0, 2, "Config mode?");
      if (!onlyMsg) {
        m.command("conf");
        m.getNotifier()->message(0, 1, "In config mode");
      }
    } break;
    case 8: {
      m.getNotifier()->message(0, 2, "Run mode?");
      if (!onlyMsg) {
        m.command("run");
        m.getNotifier()->message(0, 1, "In run mode");
      }
    } break;
    case 9: {
      m.getNotifier()->message(0, 2, "Show info?");
      if (!onlyMsg) {
        m.getNotifier()->message(0,
                                 1,
                                 "Name..:%s\nVersio:%s\nCrashe:%d\nUptime:%luh",
                                 DEVICE_NAME,
                                 STRINGIFY(PROJ_VERSION),
                                 SaveCrash.count(),
                                 (millis() / 1000) / 3600);
      }
    } break;
    default: {
      m.getNotifier()->message(0, 2, "Abort?");
      if (!onlyMsg) {
        m.getNotifier()->message(0, 1, "");
      }
    } break;
  }
  return false;
}

ICACHE_RAM_ATTR
void buttonPressed() {
  buttonInterrupts++;
  LED_INT_ON;
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

void reactCommandCustom() { // for the use via telnet
  m.command(Telnet.getLastCommand().c_str());
}

void hwTest() {

  Buffer aux(32);
  log(CLASS_MAIN, Debug, "USER INFO");
  aux.fill("VER: %s", STRINGIFY(PROJ_VERSION));
  m.getNotifier()->message(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  aux.fill("NAM: %s", DEVICE_NAME);
  m.getNotifier()->message(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  aux.fill("ID : %d", ESP.getChipId());
  m.getNotifier()->message(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  aux.fill("SSI: %s", WIFI_SSID_INIT);
  m.getNotifier()->message(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  log(CLASS_MAIN, Debug, "HW test");

#ifdef HARDWARE_TEST
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

  log(CLASS_MAIN, Debug, "..SPIFFS");
  SPIFFS.begin();
  File f = SPIFFS.open("/version.txt", "r");
  if (!f) {
    log(CLASS_MAIN, Warn, "File reading failed");
  } else {
    String s = f.readString();
    log(CLASS_MAIN, Info, "File content: %s", s.c_str());
  }
  SPIFFS.end();
  delay(HARDWARE_TEST_STEP_DELAY_MS);

#endif

  ios('r', false);
  ios('y', false);
  ios('w', false);
  ios('f', false);
  lcdImg('l', NULL);
}
