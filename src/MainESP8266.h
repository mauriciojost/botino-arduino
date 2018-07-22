
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
#include <FS.h>

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

#define LED_INT_PIN LEDW_PIN // led showing interruptions

void lcdClear(int line) {
  lcd.fillRect(0, line * 8, 128, 8, BLACK);
}

void lcdPrintLogLine(const char *logStr) {
  if (!m.getSettings()->getLcdDebug()) {
    return;
  }
  lcd.setTextWrap(false);
  lcdClear(LOG_LINE); // clear line
  lcd.setTextSize(1);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, LOG_LINE * 8);
  lcd.println(logStr);
  lcd.display();
  delay(DELAY_MS_SPI);
}

void displayUserInfo() {
  Buffer<32> aux;
  log(CLASS_MAIN, Debug, "USER INFO");
  aux.fill("VER: %s", STRINGIFY(PROJ_VERSION));
  messageFuncExt(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  aux.fill("NAM: %s", DEVICE_NAME);
  messageFuncExt(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  aux.fill("ID : %d", ESP.getChipId());
  messageFuncExt(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  aux.fill("SSI: %s", WIFI_SSID_INIT);
  messageFuncExt(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  aux.fill("PAS: %s", WIFI_PASSWORD_INIT);
  messageFuncExt(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
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

void wifiOn() {
  WiFi.mode(WIFI_STA);
  delay(WIFI_DELAY_MS);
}

void wifiOff() {
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
    delay(WIFI_DELAY_MS);
    WiFi.mode(WIFI_OFF); // to be removed after SDK update to 1.5.4
    delay(WIFI_DELAY_MS);
  }

  wifiOn();
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

void runModeArchitecture() {
  Settings *s = m.getSettings();

  // Handle stack-traces stored in memory
  if (s->getClear() && SaveCrash.count() > 0) {
    log(CLASS_MAIN, Debug, "Clearing stack-trcs");
    SaveCrash.clear();
  } else if (SaveCrash.count() > 0) {
    log(CLASS_MAIN, Warn, "Stack-trcs (!!!)");
    SaveCrash.print();
  }

  // Report interesting information about the device
  Buffer<INFO_BUFFER_LENGTH> infoBuffer;
  infoBuffer.fill("crs %d / ver %s / upt %lu h", SaveCrash.count(), STRINGIFY(PROJ_VERSION), (millis() / 1000) / 3600);
  s->setInfo(infoBuffer.getBuffer());

  // Handle log level as per settings
  Serial.setDebugOutput(s->getLogLevel() < 0); // deep HW logs

}

void logsArchitecture() {
  log(CLASS_MAIN, Info, "DEV NAME: %s", DEVICE_NAME);
  log(CLASS_MAIN, Info, "Hostname: %s", WiFi.hostname().c_str());
  log(CLASS_MAIN, Info, "IP: %s", WiFi.localIP().toString().c_str());
  log(CLASS_MAIN, Info, "Memory: %lu", ESP.getFreeHeap());
  log(CLASS_MAIN, Info, "Crashes: %d", SaveCrash.count());
  log(CLASS_MAIN, Info, "HTTP size: %d", httpClient.getSize());
}

void configureModeArchitecture() {
  initWifiSteady();
  messageFuncExt(0, 1, "telnet %s", WiFi.localIP().toString().c_str());
  Telnet.handle(); // Handle telnet log server and commands
  ArduinoOTA.handle(); // Handle on the air firmware load
  delay(DEV_USER_DELAY_MS);
}

void reactCommandCustom() { // for the use via telnet
  command(Telnet.getLastCommand().c_str());
}

bool reactToButtonHeld(int cycles, bool onlyMsg) {
	switch (cycles) {
		case 0: {
        messageFuncExt(0, 2, "Zzz routine?");
			  if (!onlyMsg) {
          log(CLASS_MAIN, Debug, "Routine Zzz...");
          m.getBody()->performMove("Zz.");
			  }
      }
      break;
		case 1:
		case 2: {
        const char* evtName = m.getIfttt()->getEventName(cycles);
        messageFuncExt(0, 2, "Push event %s?", evtName);
			  if (!onlyMsg) {
			  	bool suc = m.getIfttt()->triggerEvent(cycles);
			  	if (suc) {
            messageFuncExt(0, 1, "Event %s pushed!", evtName);
			  	} else {
            messageFuncExt(0, 1, "Failed: event not pushed");
			  	}
			  }
      }
      break;
		case 3: {
        messageFuncExt(0, 2, "Random routine?");
			  if (!onlyMsg) {
          int routine = (int)random(0, m.getSettings()->getNroRoutinesForButton());
          log(CLASS_MAIN, Debug, "Routine %d...", routine);
          m.getBody()->performMove(routine);
			  }
      }
      break;
		case 4: {
        messageFuncExt(0, 2, "All act?");
			  if (!onlyMsg) {
          messageFuncExt(0, 1, "All act one-off");
			  	for (int i=0; i < m.getBot()->getActors()->size(); i++) {
            Actor* a = m.getBot()->getActors()->get(i);
            log(CLASS_MAIN, Debug, "One off: %s", a->getName());
			  		a->oneOff();
			  	}
			  }
      }
      break;
		case 5: {
        messageFuncExt(0, 2, "LCD log?");
			  if (!onlyMsg) {
          Settings *s = m.getSettings();
          s->setLcdDebug(!s->getLcdDebug());
          messageFuncExt(0, 1, "Lcd log %d", s->getLcdDebug());
			  }
      }
      break;
		case 6: {
        messageFuncExt(0, 2, "Clear crash?");
			  if (!onlyMsg) {
			  	SaveCrash.clear();
          messageFuncExt(0, 1, "Cleared");
			  }
      }
      break;
		case 7: {
        messageFuncExt(0, 2, "Config mode?");
			  if (!onlyMsg) {
			  	m.getBot()->setMode(ConfigureMode);
          messageFuncExt(0, 1, "In config mode");
			  }
      }
      break;
		case 8: {
        messageFuncExt(0, 2, "Run mode?");
			  if (!onlyMsg) {
			  	m.getBot()->setMode(RunMode);
          messageFuncExt(0, 1, "In run mode");
			  }
      }
      break;
		case 9: {
        messageFuncExt(0, 2, "Log level?");
			  if (!onlyMsg) {
          Settings *s = m.getSettings();
          s->setLogLevel((s->getLogLevel() + 1) % 4);
          messageFuncExt(0, 1, "Log level %d", s->getLogLevel());
			  }
      }
      break;
		default:{
        messageFuncExt(0, 2, "Abort?");
			  if (!onlyMsg) {
          messageFuncExt(0, 1, "Aborted");
			  }
      }
      break;
	}
	return false;
}

bool haveToInterrupt() {
  if (Serial.available()) {
    // Handle serial commands
    Buffer<INFO_BUFFER_LENGTH> auxBuffer;
    log(CLASS_MAIN, Debug, "Serial available, listening...");
    auxBuffer.clear();
  	Serial.readBytesUntil('\n', auxBuffer.getUnsafeBuffer(), INFO_BUFFER_LENGTH);
  	auxBuffer.replace('\n', '\0');
  	auxBuffer.replace('\r', '\0');
  	command(auxBuffer.getBuffer());
  	return false;
  } else if (buttonInterrupts > 0) {
    // Handle button commands
    log(CLASS_MAIN, Debug, "Button command (%d)", buttonInterrupts);
    int holds = 0;
    delay(50); // anti-bouncing
    while(digitalRead(BUTTON0_PIN)) {
      holds++;
      log(CLASS_MAIN, Debug, "%d", holds);
      digitalWrite(LED_INT_PIN, LOW); // switch on
      reactToButtonHeld(holds, ONLY_SHOW_MSG);
      digitalWrite(LED_INT_PIN, HIGH); // switch off
      delay(950);
    }
    bool interruptMe = reactToButtonHeld(holds, SHOW_MSG_AND_REACT);
    digitalWrite(LED_INT_PIN, HIGH); // switch off
    buttonInterrupts = 0;

    log(CLASS_MAIN, Debug, "Done");
    return interruptMe;
  } else {
    // Nothing to handle, no reason to interrupt
    return false;
  }
}

void performHardwareTest() {
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

/*****************/
/*** CALLBACKS ***/
/*****************/

ICACHE_RAM_ATTR
void buttonPressed() {
  buttonInterrupts++;
  digitalWrite(LED_INT_PIN, LOW); // switch on
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

// TODO: add https support, which requires fingerprint of server that can be obtained as follows:
//  openssl s_client -connect dweet.io:443 < /dev/null 2>/dev/null | openssl x509 -fingerprint -noout -in /dev/stdin
int httpGet(const char *url, ParamStream *response) {
  httpClient.begin(url);
  httpClient.addHeader("Content-Type", "application/json");

  log(CLASS_MAIN, Debug, "> GET:..%s", tailStr(url, URL_PRINT_MAX_LENGTH));
  int errorCode = httpClient.GET();
  log(CLASS_MAIN, Debug, "> GET:%d", errorCode);

  if (errorCode == HTTP_OK) {
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

int httpPost(const char *url, const char *body, ParamStream *response) {
  httpClient.begin(url);
  httpClient.addHeader("Content-Type", "application/json");

  log(CLASS_MAIN, Debug, "> POST:..%s", tailStr(url, URL_PRINT_MAX_LENGTH));
  log(CLASS_MAIN, Debug, "> POST:'%s'", body);
  int errorCode = httpClient.POST(body);
  log(CLASS_MAIN, Debug, "> POST:%d", errorCode);

  if (errorCode == HTTP_OK) {
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
  Serial.setTimeout(10000);        // Timeout for read
  Telnet.begin("ESP" DEVICE_NAME); // Intialize the remote logging framework
  ArduinoOTA.begin();              // Intialize OTA
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

  delay(1000);
  displayUserInfo();
  performHardwareTest();

}

// TODO: buggy (what happens on overrun?), and can be simplified using the clock and time_t
void sleepInterruptable(unsigned long cycleBegin, unsigned long periodMs) {
  unsigned long spentMs = millis() - cycleBegin;
  int dc = (spentMs * 100) / periodMs;

  log(CLASS_MAIN, Info, "Disable wifi...");
  wifiOff();

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
    delay(fragToSleepMs);
    spentMs = millis() - cycleBegin;
  }
}
