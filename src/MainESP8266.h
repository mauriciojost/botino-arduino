
#include <Main.h>
#include <EspSaveCrash.h>
#include <RemoteDebug.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <Pinout.h>
#include <SPI.h>
#include <Servo.h>
#include <Wire.h>
#include <Io.h>

#define DELAY_MS_SPI 3

#define DEVICE_ALIAS_FILENAME "/device.alias"
#define DEVICE_ALIAS_MAX_LENGTH 16

#define LCD_PIXEL_WIDTH 6
#define LCD_PIXEL_HEIGHT 8

#ifndef WIFI_DELAY_MS
#define WIFI_DELAY_MS 3000
#endif // WIFI_DELAY_MS

#ifndef FIRMWARE_UPDATE_URL
#define FIRMWARE_UPDATE_URL "http://martinenhome.com:6780/firmwares/botino/latest"
#endif // FIRMWARE_UPDATE_URL

#define PRE_DEEP_SLEEP_WINDOW_FACTOR 10

#define SERVO_INVERT_POS(p, f, m) (((f) ? ((m) - (p)) : (p)))

#ifndef SERVO0_INVERTED
#define SERVO0_INVERTED false
#endif // SERVO0_INVERTED

#ifndef SERVO1_INVERTED
#define SERVO1_INVERTED false
#endif // SERVO1_INVERTED

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

#define MAX_SERVO_STEPS 10

#define SERVO0_STEP_DEGREES ((float)SERVO0_RANGE_DEGREES / (MAX_SERVO_STEPS - 1))
#define SERVO1_STEP_DEGREES ((float)SERVO1_RANGE_DEGREES / (MAX_SERVO_STEPS - 1))

#define SERVO_PERIOD_REACTION_MS 15

#define LOG_LINE 0

#ifndef URL_PRINT_MAX_LENGTH
#define URL_PRINT_MAX_LENGTH 20
#endif // URL_PRINT_MAX_LENGTH

#ifndef USER_DELAY_MS
#define USER_DELAY_MS 3000
#endif // USER_DELAY_MS

#define ONLY_SHOW_MSG true
#define SHOW_MSG_AND_REACT false

#define WAIT_BEFORE_HTTP_MS 100

extern "C" {
#include "user_interface.h"
}

#define HTTP_TIMEOUT_MS 8000

volatile unsigned char buttonInterrupts = 0;

HTTPClient httpClient;
RemoteDebug telnet;
Servo servoLeft;
Servo servoRight;
Adafruit_SSD1306 lcd(-1);
Buffer* devId = NULL;

#define LED_INT_ON ios('y', IoOn);
#define LED_INT_OFF ios('y', IoOff);

#define LED_ALIVE_ON ios('r', IoOn);
#define LED_ALIVE_OFF ios('r', IoOff);

ADC_MODE(ADC_VCC);

void bitmapToLcd(uint8_t bitmap[]);
void reactCommandCustom();
#include "MainESP8266_hwtest.h" // defines hwTest()
void buttonPressed();
void heartbeat();
void lightSleepInterruptable(time_t cycleBegin, time_t periodSecs);
void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs);
void debugHandle();
bool haveToInterrupt();

////////////////////////////////////////
// Functions requested for architecture
////////////////////////////////////////

// Callbacks
///////////////////

const char* deviceId() {
	if (devId == NULL) {
		devId = new Buffer(DEVICE_ALIAS_MAX_LENGTH);
    bool succ = readFile(DEVICE_ALIAS_FILENAME, devId); // preserve the alias
    if (succ) { // managed to retrieve the alias
    	// content already with the alias
      devId->replace('\n', 0);
    } else { // no alias, fallback to chip id
      devId->fill("%d", ESP.getChipId());
    }
    log(CLASS_MAIN, Info, "Alias '%s'", devId->getBuffer());
	}
	return devId->getBuffer();
}

void logLine(const char *str) {
  // serial print
  Serial.print("HEAP ");
  Serial.print(ESP.getFreeHeap());
  Serial.print("|");
  Serial.print("VCC ");
  Serial.print(ESP.getVcc());
  Serial.print("|");
  // telnet print
  if (telnet.isActive()) {
    Serial.print("TELNET|");
  	for (unsigned int i = 0; i < strlen(str); i++) {
      telnet.write(str[i]);
  	}
  }
  // lcd print
  if (m->getSettings()->getLcdLogs()) {
    Serial.print("LCD|");
    lcd.setTextWrap(false);
    lcd.fillRect(0, LOG_LINE * LCD_PIXEL_HEIGHT, 128, LCD_PIXEL_HEIGHT, BLACK);
    lcd.setTextSize(1);
    lcd.setTextColor(WHITE);
    lcd.setCursor(0, LOG_LINE * LCD_PIXEL_HEIGHT);
    lcd.print(str);
    lcd.display();
    delay(DELAY_MS_SPI);
  }
  Serial.print(str);
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

void messageFunc(int x, int y, int color, bool wrap, MsgClearMode clearMode, int size, const char *str) {
  int l = strlen(str);
  switch (clearMode) {
  	case FullClear:
      lcd.clearDisplay();
      break;
  	case LineClear:
      lcd.fillRect(x * size * LCD_PIXEL_WIDTH, y * size * LCD_PIXEL_HEIGHT, 128, size * LCD_PIXEL_HEIGHT, !color);
      wrap = false;
      break;
  	case NoClear:
      break;
  }
  lcd.setTextWrap(wrap);
  lcd.setTextSize(size);
  lcd.setTextColor(color);
  lcd.setCursor(x * size * LCD_PIXEL_WIDTH, y * size * LCD_PIXEL_HEIGHT);
  lcd.print(str);
  lcd.display();
  log(CLASS_MAIN, Debug, "Msg: (%d,%d)'%s'", x, y, str);
  delay(DELAY_MS_SPI);
}

void arms(int left, int right, int steps) {
  static int lastPosL = -1;
  static int lastPosR = -1;

  log(CLASS_MAIN, Debug, "Arms>%d&%d", left, right);
  int targetPosL = SERVO0_BASE_DEGREES + SERVO_INVERT_POS(((POSIT(left) % MAX_SERVO_STEPS) * SERVO0_STEP_DEGREES) , SERVO0_INVERTED, SERVO0_RANGE_DEGREES);
  int targetPosR = SERVO1_BASE_DEGREES + SERVO_INVERT_POS(((POSIT(right) % MAX_SERVO_STEPS) * SERVO1_STEP_DEGREES), SERVO1_INVERTED, SERVO1_RANGE_DEGREES);
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
    delay(SERVO_PERIOD_REACTION_MS);
  }
  lastPosL = targetPosL;
  lastPosR = targetPosR;
  servoLeft.detach();
  servoRight.detach();
}

IoMode invert(IoMode v) {
	if (v == IoOn) {
		return IoOff;
	} else if (v == IoOff) {
		return IoOn;
	} else {
		return v;
	}
}

void ios(char led, IoMode value) {
	uint8_t pin = -1;
  switch (led) {
    case 'r':
      pin = LEDR_PIN;
      break;
    case 'w':
      pin = LEDW_PIN;
      break;
    case 'y':
      pin = LEDY_PIN;
      break;
    case 'f':
      pin = FAN_PIN;
      value = invert(value);
      break;
    default:
    	pin = -1;
      break;
  }
  if (pin == -1) {
    log(CLASS_MAIN, Warn, "Unknown pin: %c", pin);
  	return;
  }
  if (value == IoOn) {
  	digitalWrite(pin, LOW);
  } else if (value == IoOff) {
  	digitalWrite(pin, HIGH);
  } else if (value == IoToggle){
  	digitalWrite(pin, digitalRead(pin));
  } else {
    log(CLASS_MAIN, Warn, "Unknown IO request: %c<-%d", pin, value);
  }

}

void clearDevice() {
	Buffer alias(DEVICE_ALIAS_MAX_LENGTH);
  readFile(DEVICE_ALIAS_FILENAME, &alias); // preserve the alias
  SPIFFS.format();
  SaveCrash.clear();
  writeFile(DEVICE_ALIAS_FILENAME, alias.getBuffer());
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

bool readFile(const char* fname, Buffer* content) {
	bool success = false;
  SPIFFS.begin();
  File f = SPIFFS.open(fname, "r");
  if (!f) {
    log(CLASS_MAIN, Warn, "File reading failed: %s", fname);
    content->clear();
    success = false;
  } else {
    String s = f.readString();
    content->load(s.c_str());
    log(CLASS_MAIN, Info, "File read: %s", fname);
    success = true;
  }
  SPIFFS.end();
  return success;
}

bool writeFile(const char* fname, const char* content) {
	bool success = false;
  SPIFFS.begin();
  File f = SPIFFS.open(fname, "w+");
  if (!f) {
    log(CLASS_MAIN, Warn, "File writing failed: %s", fname);
    success = false;
  } else {
    f.write((const uint8_t*)content, strlen(content));
    log(CLASS_MAIN, Info, "File written: %s", fname);
    success = true;
  }
  SPIFFS.end();
  return success;
}

void infoArchitecture() {

  m->getNotifier()->message(0,
                           1,
                           "DEV ID:%s\nVers:%s\nCrashes:%d\nIP: %s\nMemory:%lu\nUptime:%luh\nVcc: %0.2f",
                           deviceId(),
                           STRINGIFY(PROJ_VERSION),
                           SaveCrash.count(),
                           WiFi.localIP().toString().c_str(),
                           ESP.getFreeHeap(),
                           (millis() / 1000) / 3600,
                           ((float)ESP.getVcc()/1024));

}

void testArchitecture() {
  hwTest();
}

void updateFirmware() {
  ESP8266HTTPUpdate updater;
  log(CLASS_MAIN, Info, "Updating firmware from '%s'...", FIRMWARE_UPDATE_URL);
  t_httpUpdate_return ret = updater.update(FIRMWARE_UPDATE_URL);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      log(CLASS_MAIN, Error, "HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      log(CLASS_MAIN, Info, "HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      log(CLASS_MAIN, Info, "HTTP_UPDATE_OK");
      break;
  }
}

// Execution
///////////////////

void sleepInterruptable(time_t cycleBegin, time_t periodSecs) {
	if (m->getSettings()->inDeepSleepMode() && periodSecs > 120) { // in deep sleep mode and period big enough
		m->command("move Z.");
		lightSleepInterruptable(now() /* always do it */, periodSecs / PRE_DEEP_SLEEP_WINDOW_FACTOR);
		m->command("move Z.");
		deepSleepNotInterruptable(cycleBegin, periodSecs);
	} else {
		lightSleepInterruptable(cycleBegin, periodSecs);
	}
}

BotMode setupArchitecture() {

  // Let HW startup
  delay(2 * 1000);

  // Setup pins
  log(CLASS_MAIN, Debug, "Setup pins");
  pinMode(LEDR_PIN, OUTPUT);
  pinMode(LEDW_PIN, OUTPUT);
  pinMode(LEDY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(SERVO0_PIN, OUTPUT);
  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(BUTTON0_PIN, INPUT);

  // Intialize the logging framework
  Serial.begin(115200);     // Initialize serial port
  Serial.setTimeout(10000); // Timeout for read
  lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(DELAY_MS_SPI); // Initialize LCD
  setupLog(logLine);   // Initialize log callback
  heartbeat();

  log(CLASS_MAIN, Debug, "Setup timing");
  setExternalMillis(millis);

  log(CLASS_MAIN, Debug, "Setup wdt");
  ESP.wdtEnable(1); // argument not used

  log(CLASS_MAIN, Debug, "Setup wifi");
  WiFi.persistent(false);
  WiFi.hostname(deviceId());
  heartbeat();

  log(CLASS_MAIN, Debug, "Setup http");
  httpClient.setTimeout(HTTP_TIMEOUT_MS);
  heartbeat();

  log(CLASS_MAIN, Debug, "Setup random");
  randomSeed(analogRead(0) * 256 + analogRead(0));
  heartbeat();

  log(CLASS_MAIN, Debug, "Setup interrupts");
  attachInterrupt(digitalPinToInterrupt(BUTTON0_PIN), buttonPressed, RISING);
  heartbeat();

  log(CLASS_MAIN, Debug, "Setup commands");
  telnet.setCallBackProjectCmds(reactCommandCustom);
  String helpCli(HELP_COMMAND_CLI);
  telnet.setHelpProjectsCmds(helpCli);
  heartbeat();

  log(CLASS_MAIN, Debug, "Setup IO/lcd");
  ios('r', IoOff);
  ios('y', IoOff);
  ios('w', IoOff);
  ios('f', IoOff);
  lcdImg('l', NULL);
  heartbeat();

  if (digitalRead(BUTTON0_PIN)) {
    return ConfigureMode;
  } else {
    return RunMode;
  }
}

void runModeArchitecture() {
  Settings *s = m->getSettings();

  // Handle stack-traces stored in memory
  if (SaveCrash.count() > 5) {
    log(CLASS_MAIN, Warn, "Too many Stack-trcs / clearing (!!!)");
    SaveCrash.clear();
  } else if (SaveCrash.count() > 0) {
    log(CLASS_MAIN, Warn, "Stack-trcs (!!!)");
    SaveCrash.print();
  }

  // Handle log level as per settings
  Serial.setDebugOutput(s->getDebug()); // deep HW logs

  if (s->getDebug()) {
    debugHandle();
  }
}

void configureModeArchitecture() {
	debugHandle();
	if (m->getBot()->getClock()->currentTime() % 60 == 0) { // every minute
    m->getNotifier()->message(0, 1, "telnet %s", WiFi.localIP().toString().c_str());
	}
}

void abort(const char* msg) {
  log(CLASS_MAIN, Error, "Abort: %s", msg);
#ifdef DEEP_SLEEP_MODE_ENABLED
  ESP.deepSleep(60 * 1000000L); // reboot in a while
#else // DEEP_SLEEP_MODE_ENABLED
  ESP.restart(); // restart right away
#endif // DEEP_SLEEP_MODE_ENABLED
}

////////////////////////////////////////
// Architecture specific functions
////////////////////////////////////////

void debugHandle() {
  static bool firstTime = true;
  if (firstTime) {
    log(CLASS_MAIN, Debug, "Initialize debuggers...");
    telnet.begin(deviceId());        // Intialize the remote logging framework
    ArduinoOTA.begin();              // Intialize OTA
    firstTime = false;
  }
  telnet.handle();     // Handle telnet log server and commands
  ArduinoOTA.handle(); // Handle on the air firmware load
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
  m->command(telnet.getLastCommand().c_str());
}

void heartbeat() {
  LED_ALIVE_ON
  delay(1);
  LED_ALIVE_OFF
}

void lightSleepInterruptable(time_t cycleBegin, time_t periodSecs) {
  log(CLASS_MAIN, Debug, "Light Sleep(%ds)...", (int)periodSecs);
  while (now() < cycleBegin + periodSecs) {
    if (haveToInterrupt()) {
      break;
    }
    delay(m->getSettings()->miniPeriodMsec());
  }
}

void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs) {
  log(CLASS_MAIN, Debug, "Deep Sleep(%ds)...", (int)periodSecs);
  time_t spentSecs = now() - cycleBegin;
  time_t leftSecs = periodSecs - spentSecs;
  if (leftSecs > 0) {
    ESP.deepSleep(leftSecs * 1000000L);
  }
}

bool haveToInterrupt() {
  heartbeat();
  if (Serial.available()) {
    // Handle serial commands
    Buffer cmdBuffer(COMMAND_MAX_LENGTH);
    log(CLASS_MAIN, Info, "Listening...");
    cmdBuffer.clear();
    Serial.readBytesUntil('\n', cmdBuffer.getUnsafeBuffer(), COMMAND_MAX_LENGTH);
    cmdBuffer.replace('\n', '\0');
    cmdBuffer.replace('\r', '\0');
    bool interrupt = m->command(cmdBuffer.getBuffer());
    log(CLASS_MAIN, Debug, "Interrupt: %d", interrupt);
    return interrupt;
  } else if (buttonInterrupts > 0) {
    // Handle button commands
    log(CLASS_MAIN, Debug, "Button command (%d)", buttonInterrupts);
    int holds = -1;
    do {
      holds++;
      log(CLASS_MAIN, Debug, "%d", holds);
      LED_INT_ON;
      m->sequentialCommand(holds, ONLY_SHOW_MSG);
      LED_INT_OFF;
      delay(m->getSettings()->miniPeriodMsec());
    } while (digitalRead(BUTTON0_PIN));
    bool interruptMe = m->sequentialCommand(holds, SHOW_MSG_AND_REACT);
    buttonInterrupts = 0;

    log(CLASS_MAIN, Debug, "Done");
    return interruptMe;
  } else {
    // Nothing to handle, no reason to interrupt
    return false;
  }
}
