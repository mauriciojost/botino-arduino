
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
//#include <ESP32WiFi.h>
#include <HTTPUpdate.h>
//#include <EspSaveCrash.h>
#include <FS.h>
#include <Main.h>
#include <Pinout.h>
#include <RemoteDebug.h>
#include <SPI.h>
//#include <Servo.h>
#include <Wire.h>
#include <utils/Io.h>
//#include <utils/ServoConf.h>

#define FORMAT_SPIFFS_IF_FAILED true

#define DELAY_MS_SPI 3
#define ABORT_DELAY_SECS 5
#define HW_STARTUP_DELAY_MSECS 500

#define DEVICE_ALIAS_FILENAME "/alias.tuning"
#define DEVICE_ALIAS_MAX_LENGTH 16

#define DEVICE_PWD_FILENAME "/pass.tuning"
#define DEVICE_PWD_MAX_LENGTH 16

//#define SERVO_0_FILENAME "/servo0.tuning"
//#define SERVO_1_FILENAME "/servo1.tuning"

#define LCD_PIXEL_WIDTH 6
#define LCD_PIXEL_HEIGHT 8

#ifndef WIFI_DELAY_MS
#define WIFI_DELAY_MS 2000
#endif // WIFI_DELAY_MS

#define MAX_ROUND_ROBIN_LOG_FILES 5

#ifndef FIRMWARE_UPDATE_URL
#define FIRMWARE_UPDATE_URL MAIN4INOSERVER_API_HOST_BASE "/firmwares/botino/%s"
#endif // FIRMWARE_UPDATE_URL

#define PRE_DEEP_SLEEP_WINDOW_SECS 5

#define SERVO_BASE_STEPS 120
#define SERVO_PERIOD_STEP_MS 2

#define NEXT_LOG_LINE_ALGORITHM ((currentLogLine + 1) % 4)

#define LOG_BUFFER_MAX_LENGTH 1024

#ifndef URL_PRINT_MAX_LENGTH
#define URL_PRINT_MAX_LENGTH 20
#endif // URL_PRINT_MAX_LENGTH

#ifndef USER_DELAY_MS
#define USER_DELAY_MS 8000
#endif // USER_DELAY_MS

#define USER_LCD_FONT_SIZE 2

//#define VCC_FLOAT ((float)ESP.getVcc() / 1024)

#define ONLY_SHOW_MSG true
#define SHOW_MSG_AND_REACT false

#define WAIT_BEFORE_HTTP_MS 100

//extern "C" {
//#include "user_interface.h"
//}

#define HTTP_TIMEOUT_MS 8000

#define HELP_COMMAND_ARCH_CLI                                                                                                              \
  "\n  ESP32 HELP"                                                                                                                        \
  "\n  init              : initialize essential settings (wifi connection, logins, etc.)"                                                  \
  "\n  servo ...         : tune the servo <s> (r|l) and make a test round "                                                                \
  "\n  rm ...            : remove file in FS "                                                                                             \
  "\n  ls                : list files present in FS "                                                                                      \
  "\n  reset             : reset the device"                                                                                               \
  "\n  deepsleep ...     : deep sleep N provided seconds"                                                                                  \
  "\n  lightsleep ...    : light sleep N provided seconds"                                                                                 \
  "\n  clearstack        : clear stack trace "                                                                                             \
  "\n"

#define BUTTON_IS_PRESSED ((bool)digitalRead(BUTTON0_PIN))

volatile bool buttonEnabled = true;
volatile unsigned char buttonInterrupts = 0;

HTTPClient httpClient;
RemoteDebug telnet;
//Servo servoLeft;
//Servo servoRight;
Adafruit_SSD1306 *lcd = NULL;
Buffer *apiDeviceId = NULL;
Buffer *apiDevicePwd = NULL;
Buffer *deepSleepMode = NULL;
//ServoConf *servo0Conf = NULL;
//ServoConf *servo1Conf = NULL;
int currentLogLine = 0;
Buffer *logBuffer = NULL;

#define LED_INT_TOGGLE ios('w', IoToggle);
#define LED_INT_ON ios('w', IoOn);
#define LED_ALIVE_TOGGLE ios('r', IoToggle);

//ADC_MODE(ADC_VCC);

void bitmapToLcd(uint8_t bitmap[]);
void reactCommandCustom();
#include "Main_hwtest.h" // defines hwTest()
void buttonPressed();
void heartbeat();
bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs);
void debugHandle();
bool haveToInterrupt();
void handleInterrupt();
//void initializeServoConfigs();
Buffer *initializeTuningVariable(Buffer **var, const char *filename, int maxLength, const char *defaultContent, bool obfuscate);
void dumpLogBuffer();

////////////////////////////////////////
// Functions requested for architecture
////////////////////////////////////////

// Callbacks
///////////////////

const char *apiDeviceLogin() {
  return initializeTuningVariable(&apiDeviceId, DEVICE_ALIAS_FILENAME, DEVICE_ALIAS_MAX_LENGTH, NULL, false)->getBuffer();
}

const char *apiDevicePass() {
  return initializeTuningVariable(&apiDevicePwd, DEVICE_PWD_FILENAME, DEVICE_PWD_MAX_LENGTH, NULL, true)->getBuffer();
}

void logLine(const char *str) {
  Serial.setDebugOutput(getLogLevel() == Debug); // deep HW logs
  // serial print
	/*
  Serial.print("HEA:");
  Serial.print(ESP.getFreeHeap());
  Serial.print("|");
  Serial.print("VCC:");
  Serial.print(VCC_FLOAT);
  Serial.print("|");
  */
  Serial.print(str);
  // telnet print
  if (telnet.isActive()) {
    for (unsigned int i = 0; i < strlen(str); i++) {
      telnet.write(str[i]);
    }
  }
  // lcd print
  if (lcd != NULL && m->getBotinoSettings()->getLcdLogs()) { // can be called before LCD initialization
    currentLogLine = NEXT_LOG_LINE_ALGORITHM;
    lcd->setTextWrap(false);
    lcd->fillRect(0, currentLogLine * LCD_PIXEL_HEIGHT, 128, LCD_PIXEL_HEIGHT, BLACK);
    lcd->setTextSize(1);
    lcd->setTextColor(WHITE);
    lcd->setCursor(0, currentLogLine * LCD_PIXEL_HEIGHT);
    lcd->print(str);
    lcd->display();
    delay(DELAY_MS_SPI);
  }
  // filesystem logs
  if (m->getBotinoSettings()->fsLogsEnabled()) {
    if (logBuffer == NULL) {
      logBuffer = new Buffer(LOG_BUFFER_MAX_LENGTH);
    }
    logBuffer->append(str);
    logBuffer->append("\n");
  }
}

void stopWifi() {
  log(CLASS_MAIN, Info, "W.Off.");
  WiFi.disconnect();
  delay(WIFI_DELAY_MS);
  WiFi.mode(WIFI_OFF); // to be removed after SDK update to 1.5.4
  delay(WIFI_DELAY_MS);
}

bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries) {
  wl_status_t status;
  log(CLASS_MAIN, Info, "To '%s'...", ssid);

  if (skipIfConnected) { // check if connected
    log(CLASS_MAIN, Info, "Conn. '%s'?", ssid);
    status = WiFi.status();
    if (status == WL_CONNECTED) {
      log(CLASS_MAIN, Info, "IP: %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
  } else { // force disconnection
  	stopWifi();
  }

  WiFi.mode(WIFI_STA);
  delay(WIFI_DELAY_MS);
  WiFi.begin(ssid, pass);

  int attemptsLeft = retries;
  while (true) {
    bool interrupt = lightSleepInterruptable(now(), WIFI_DELAY_MS / 1000);
    if (interrupt) {
      log(CLASS_MAIN, Info, "Interrupted");
      return false; // not connected
    }
    status = WiFi.status();
    log(CLASS_MAIN, Info, "..'%s'(%d)", ssid, attemptsLeft);
    attemptsLeft--;
    if (status == WL_CONNECTED) {
      log(CLASS_MAIN, Info, "IP: %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
    if (attemptsLeft < 0) {
      log(CLASS_MAIN, Warn, "Conn. to '%s' failed %d", ssid, status);
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
  switch (clearMode) {
    case FullClear:
      lcd->clearDisplay();
      break;
    case LineClear:
      lcd->fillRect(x * size * LCD_PIXEL_WIDTH, y * size * LCD_PIXEL_HEIGHT, 128, size * LCD_PIXEL_HEIGHT, !color);
      wrap = false;
      break;
    case NoClear:
      break;
  }
  lcd->setTextWrap(wrap);
  lcd->setTextSize(size);
  lcd->setTextColor(color);
  lcd->setCursor(x * size * LCD_PIXEL_WIDTH, y * size * LCD_PIXEL_HEIGHT);
  lcd->print(str);
  lcd->display();
  log(CLASS_MAIN, Debug, "Msg: (%d,%d)'%s'", x, y, str);
  delay(DELAY_MS_SPI);
}

void arms(int left, int right, int periodFactor) {
	/*
  static int lastDegL = -1;
  static int lastDegR = -1;

  int steps = periodFactor * SERVO_BASE_STEPS;

  log(CLASS_MAIN, Debug, "Arms>%d&%d", left, right);

  int targetDegL = servo0Conf->getTargetDegreesFromPosition(left);
  servoLeft.attach(SERVO0_PIN);

  int targetDegR = servo1Conf->getTargetDegreesFromPosition(right);
  servoRight.attach(SERVO1_PIN);

  // leave as target if first time
  lastDegL = (lastDegL == -1 ? targetDegL : lastDegL);
  lastDegR = (lastDegR == -1 ? targetDegR : lastDegR);

  log(CLASS_MAIN, Debug, "Sv.Ldeg%d>deg%d", lastDegL, targetDegL);
  log(CLASS_MAIN, Debug, "Sv.Rdeg%d>deg%d", lastDegR, targetDegR);
  for (int i = 1; i <= steps; i++) {
    float factor = ((float)i) / steps;
    int vL = lastDegL + ((targetDegL - lastDegL) * factor);
    int vR = lastDegR + ((targetDegR - lastDegR) * factor);
    servoLeft.write(vL);
    servoRight.write(vR);
    delay(SERVO_PERIOD_STEP_MS);
  }
  lastDegL = targetDegL;
  lastDegR = targetDegR;
  servoLeft.detach();
  servoRight.detach();
  */
}

void ios(char led, IoMode value) {
  uint8_t pin = -1;
  switch (led) {
    case '*':
      ios('r', value);
      ios('w', value);
      ios('y', value);
      ios('f', value);
      return;
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
  } else if (value == IoToggle) {
    digitalWrite(pin, !digitalRead(pin));
  } else {
    log(CLASS_MAIN, Warn, "Unknown IO request: %c<-%d", pin, value);
  }
}

void clearDevice() {
  SPIFFS.format();
  //SaveCrash.clear();
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "Img '%c'", img);
  switch (img) {
    case '_': // dim
      log(CLASS_MAIN, Debug, "Dim face");
      lcd->dim(true);
      break;
    case '-': // bright
      log(CLASS_MAIN, Debug, "Bright face");
      lcd->dim(false);
      break;
    case 'w': // white
      log(CLASS_MAIN, Debug, "White face");
      lcd->invertDisplay(true);
      break;
    case 'b': // black
      log(CLASS_MAIN, Debug, "Black face");
      lcd->invertDisplay(false);
      break;
    case 'l': // clear
      log(CLASS_MAIN, Debug, "Clear face");
      lcd->invertDisplay(false);
      lcd->clearDisplay();
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
  lcd->display();
  delay(DELAY_MS_SPI);
}

bool readFile(const char *fname, Buffer *content) {
  bool success = false;
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
  return success;
}

bool writeFile(const char *fname, const char *content) {
  bool success = false;
  File f = SPIFFS.open(fname, "w+");
  if (!f) {
    log(CLASS_MAIN, Warn, "File writing failed: %s", fname);
    success = false;
  } else {
    f.print(content);
    f.close();
    log(CLASS_MAIN, Info, "File written: %s", fname);
    success = true;
  }
  return success;
}

void infoArchitecture() {

  m->getNotifier()->message(0,
                            1,
                            "ID:%s\nV:%s\nIP: %s\nMemory:%lu\nUptime:%luh\n",
                            apiDeviceLogin(),
                            STRINGIFY(PROJ_VERSION),
                            WiFi.localIP().toString().c_str(),
                            ESP.getFreeHeap(),
                            (millis() / 1000) / 3600);
}

void testArchitecture() {
  hwTest();
}

void updateFirmware(const char* descriptor) {
  HTTPUpdate updater;
  Buffer url(64);
  url.fill(FIRMWARE_UPDATE_URL, descriptor);

  Settings *s = m->getModuleSettings();
  bool connected = initWifi(s->getSsid(), s->getPass(), false, 10);
  if (!connected) {
    log(CLASS_MAIN, Error, "Cannot connect to wifi");
    m->getNotifier()->message(0, USER_LCD_FONT_SIZE, "Cannot connect to wifi: %s", s->getSsid());
    return; // fail fast
  }

  log(CLASS_MAIN, Info, "Updating firmware from '%s'...", url.getBuffer());
  m->getNotifier()->message(0, USER_LCD_FONT_SIZE, "Updating: %s", url.getBuffer());

  t_httpUpdate_return ret = updater.update(httpClient.getStream(), url.getBuffer(), STRINGIFY(PROJ_VERSION));
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      log(CLASS_MAIN,
          Error,
          "HTTP_UPDATE_FAILD Error (%d): %s\n",
          updater.getLastError(),
          updater.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      log(CLASS_MAIN, Info, "No updates.");
      break;
    case HTTP_UPDATE_OK:
      log(CLASS_MAIN, Info, "Done!");
      break;
  }
}

// Execution
///////////////////

bool sleepInterruptable(time_t cycleBegin, time_t periodSecs) {
  return lightSleepInterruptable(cycleBegin, periodSecs);
}

BotMode setupArchitecture() {

  // Let HW startup
  delay(HW_STARTUP_DELAY_MSECS);

  // Intialize the logging framework
  Serial.begin(115200);     // Initialize serial port
  Serial.setTimeout(10000); // Timeout for read
  setupLog(logLine);
  log(CLASS_MAIN, Info, "Log initialized");

  log(CLASS_MAIN, Debug, "Setup timing");
  setExternalMillis(millis);

  log(CLASS_MAIN, Debug, "Setup SPIFFS");
  SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);

  log(CLASS_MAIN, Debug, "Setup pins & deepsleep (if failure think of activating deep sleep mode?)");
  pinMode(LEDR_PIN, OUTPUT);
  pinMode(LEDW_PIN, OUTPUT);
  pinMode(LEDY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(SERVO0_PIN, OUTPUT);
  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(BUTTON0_PIN, INPUT);

  log(CLASS_MAIN, Debug, "Setup LCD");
  lcd = new Adafruit_SSD1306(-1);
  lcd->begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize LCD
  delay(DELAY_MS_SPI);
  heartbeat();

  //log(CLASS_MAIN, Debug, "Setup wdt");
  //ESP.wdtEnable(1); // argument not used

  /*
  log(CLASS_MAIN, Debug, "Setup wifi");
  WiFi.persistent(false);
  WiFi.hostname(apiDeviceLogin());
  heartbeat();
  */

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
  String helpCli("Type 'help' for help");
  telnet.setHelpProjectsCmds(helpCli);
  heartbeat();

  log(CLASS_MAIN, Debug, "Setup IO/lcd");
  ios('*', IoOff);
  lcdImg('l', NULL);
  heartbeat();


  //log(CLASS_MAIN, Debug, "Setup servos");
  //initializeServoConfigs();

  /*
  log(CLASS_MAIN, Debug, "Clean up crashes");
  if (SaveCrash.count() > 5) {
    log(CLASS_MAIN, Warn, "Too many Stack-trcs / clearing (!!!)");
    SaveCrash.clear();
  } else if (SaveCrash.count() > 0) {
    log(CLASS_MAIN, Warn, "Stack-trcs (!!!)");
    SaveCrash.print();
  }
  */

  return RunMode;
}

void runModeArchitecture() {
  handleInterrupt();
  if (m->getModuleSettings()->getDebug()) {
    debugHandle();
  }
}

void askStringQuestion(const char *question, Buffer *answer) {
  m->getNotifier()->message(0, USER_LCD_FONT_SIZE, "%s\n(answer serial and enter)", question);
  Serial.readBytesUntil('\n', answer->getUnsafeBuffer(), COMMAND_MAX_LENGTH);
  answer->replace('\n', '\0');
  answer->replace('\r', '\0');
}

bool askBoolQuestion(const char *question) {
  m->getNotifier()->message(0, USER_LCD_FONT_SIZE, "%s\n(Press if true)", question);
  delay(USER_DELAY_MS);
  int answer = BUTTON_IS_PRESSED;
  return (bool)answer;
}

/*
void tuneServo(const char *name, int pin, Servo *servo, ServoConf *servoConf) {
  servo->attach(pin);
  servo->write(0);

  m->getNotifier()->message(0, USER_LCD_FONT_SIZE, "Tuning\n%s", name);
  delay(USER_DELAY_MS);

  int min = 100;
  int max = 100;
  int testRange = 200;

  m->getNotifier()->message(0, USER_LCD_FONT_SIZE, "Press\nbutton\nif arm\nmoves...");
  delay(SERVO_PERIOD_STEP_MS * SERVO_BASE_STEPS);
  delay(USER_DELAY_MS);

  for (int d = 0; d <= testRange; d = d + 2) {
    bool pressed = BUTTON_IS_PRESSED;
    log(CLASS_MODULE, Info, "Moves: %d/%d %s", d, testRange, (pressed?"<= IN RANGE":""));
    min = ((d < min) && pressed ? d : min);
    max = ((d > max) && pressed ? d : max);
    servo->write(d);
    delay(SERVO_PERIOD_STEP_MS * SERVO_BASE_STEPS);
  }

  bool dwn = askBoolQuestion("Is the arm\ndown now?");
  m->getNotifier()->message(0, USER_LCD_FONT_SIZE, "Arm now:\n%s", (dwn?"DOWN":"UP"));
  delay(USER_DELAY_MS);
  m->getNotifier()->message(0, 2, "Setup\n%s\ndone!", name);
  delay(USER_DELAY_MS);

  servoConf->setBase(min);
  servoConf->setRange(max - min);
  servoConf->setInvert(dwn);

  servo->detach();
}
*/

CmdExecStatus commandArchitecture(const char *c) {
  if (strcmp("servo", c) == 0) {
  	/*
    char servo = strtok(NULL, " ")[0];
    Buffer serialized(16);
    if (servo == 'r' || servo == 'R') {
      arms(2, 2, 1);
      arms(2, 7, 1);
      arms(2, 2, 1);
      tuneServo("right servo", SERVO1_PIN, &servoRight, servo1Conf);
      servo1Conf->serialize(&serialized); // right servo1
      writeFile(SERVO_1_FILENAME, serialized.getBuffer());
      logUser("Stored tuning right servo");
      arms(0, 0, 1);
      arms(0, 9, 1);
      arms(0, 0, 1);
      return Executed;
    } else if (servo == 'l' || servo == 'L') {
      arms(2, 2, 1);
      arms(7, 2, 1);
      arms(2, 2, 1);
      tuneServo("left servo", SERVO0_PIN, &servoLeft, servo0Conf);
      servo0Conf->serialize(&serialized); // left servo0
      writeFile(SERVO_0_FILENAME, serialized.getBuffer());
      logUser("Stored tuning left servo");
      arms(0, 0, 1);
      arms(9, 0, 1);
      arms(0, 0, 1);
      return Executed;
    } else {
      logUser("Invalid servo (l|r)");
      return InvalidArgs;
    }
      */
  	return InvalidArgs;
  } else if (strcmp("init", c) == 0) {
    logRawUser("-> Initialize");
    logRawUser("Execute:");
    logRawUser("   ls");
    logUser("   save %s <alias>", DEVICE_ALIAS_FILENAME);
    logUser("   save %s <pwd>", DEVICE_PWD_FILENAME);
    //logRawUser("   servo l");
    //logRawUser("   servo r");
    logRawUser("   wifissid <ssid>");
    logRawUser("   wifissid <ssid>");
    logRawUser("   wifipass <password>");
    logRawUser("   ifttttoken <token>");
    logRawUser("   (setup of power consumption settings architecture specific if any)");
    logRawUser("   store");
    logRawUser("   ls");
    return Executed;
  } else if (strcmp("ls", c) == 0) {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file) {
      logUser("- %s (%d bytes)", file.name(), (int)file.size());
      file = root.openNextFile();
    }
    return Executed;
  } else if (strcmp("rm", c) == 0) {
    const char *f = strtok(NULL, " ");
    bool succ = SPIFFS.remove(f);
    logUser("### File '%s' %s removed", f, (succ?"":"NOT"));
    return Executed;
  } else if (strcmp("reset", c) == 0) {
    ESP.restart(); // it is normal that it fails if invoked the first time after firmware is written
    return Executed;
  } else if (strcmp("deepsleep", c) == 0) {
    int s = atoi(strtok(NULL, " "));
    deepSleepNotInterruptable(now(), s);
    return Executed;
  } else if (strcmp("lightsleep", c) == 0) {
    int s = atoi(strtok(NULL, " "));
    return (lightSleepInterruptable(now(), s)? ExecutedInterrupt: Executed);
  } else if (strcmp("clearstack", c) == 0) {
    //SaveCrash.clear();
    return Executed;
  } else if (strcmp("help", c) == 0 || strcmp("?", c) == 0) {
    logRawUser(HELP_COMMAND_ARCH_CLI);
    return Executed;
  } else {
    return NotFound;
  }
}

void configureModeArchitecture() {
  handleInterrupt();
  debugHandle();
  if (m->getBot()->getClock()->currentTime() % 60 == 0) { // every minute
    m->getNotifier()->message(0, 2, "telnet %s", WiFi.localIP().toString().c_str());
  }
}

void abort(const char *msg) {
  log(CLASS_MAIN, Error, "Abort: %s", msg);
  m->getNotifier()->message(0, 2, "Abort: %s", msg);
  bool interrupt = sleepInterruptable(now(), ABORT_DELAY_SECS);
  if (interrupt) {
    log(CLASS_MAIN, Debug, "Abort sleep interrupted");
  } else {
    log(CLASS_MAIN, Warn, "Will light sleep and restart upon abort...");
    bool i = sleepInterruptable(now(), m->getModuleSettings()->periodMsec() / 1000L);
    if (!i) {
      ESP.restart();
    } else {
      log(CLASS_MAIN, Warn, "Restart skipped because of interrupt.");
      log(CLASS_MAIN, Warn, "System ready for exploration.");
    }
  }
}

////////////////////////////////////////
// Architecture specific functions
////////////////////////////////////////

void debugHandle() {
  static bool firstTime = true;
  if (firstTime) {
    log(CLASS_MAIN, Debug, "Initialize debuggers...");
    telnet.begin(apiDeviceLogin()); // Intialize the remote logging framework
    ArduinoOTA.begin();             // Intialize OTA
    firstTime = false;
  }

  m->getBotinoSettings()->getStatus()->fill("heap:%d", ESP.getFreeHeap());
  m->getBotinoSettings()->getMetadata()->changed();

  if (m->getBotinoSettings()->fsLogsEnabled()) {
    dumpLogBuffer();
  }
  telnet.handle();     // Handle telnet log server and commands
  ArduinoOTA.handle(); // Handle on the air firmware load
}

ICACHE_RAM_ATTR
void buttonPressed() {
  if (buttonEnabled) {
    buttonInterrupts++;
  }
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
        lcd->fillRect(xl, yl, 8, 8, cl);
      }
    }
  }
}

void reactCommandCustom() { // for the use via telnet
  m->command(telnet.getLastCommand().c_str());
}

void heartbeat() {
  LED_ALIVE_TOGGLE
  delay(1);
  LED_ALIVE_TOGGLE
}

bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs) {
  log(CLASS_MAIN, Debug, "Light Sleep(%ds)...", (int)periodSecs);
  if (haveToInterrupt()) { // first quick check before any time considerations
    return true;
  }
  while (now() < cycleBegin + periodSecs) {
    if (haveToInterrupt()) {
      return true;
    }
    heartbeat();
    delay(m->getModuleSettings()->miniPeriodMsec());
  }
  return false;
}

void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs) {
	lightSleepInterruptable(cycleBegin, periodSecs);
}

void handleInterrupt() {
  if (Serial.available()) {
    // Handle serial commands
    Buffer cmdBuffer(COMMAND_MAX_LENGTH);
    log(CLASS_MAIN, Info, "Listening...");
    Serial.readBytesUntil('\n', cmdBuffer.getUnsafeBuffer(), COMMAND_MAX_LENGTH);
    cmdBuffer.replace('\n', 0);
    cmdBuffer.replace('\r', 0);
    CmdExecStatus execStatus = m->command(cmdBuffer.getBuffer());
    bool interrupt = (execStatus == ExecutedInterrupt);
    log(CLASS_MAIN, Debug, "Interrupt: %d", interrupt);
    log(CLASS_MAIN, Debug, "Cmd status: %s", CMD_EXEC_STATUS(execStatus));
    logUser("(%s => %s)", cmdBuffer.getBuffer(), CMD_EXEC_STATUS(execStatus));
  } else if (buttonInterrupts > 0) {
    buttonEnabled = false;
    buttonInterrupts = 0; // to avoid interrupting whatever is called below
    int holds = -1;
    do {
      holds++;
      log(CLASS_MAIN, Debug, "%d", holds);
      LED_INT_TOGGLE;
      m->sequentialCommand(holds, ONLY_SHOW_MSG);
      LED_INT_TOGGLE;
      delay(m->getModuleSettings()->miniPeriodMsec());
    } while (BUTTON_IS_PRESSED);
    m->sequentialCommand(holds, SHOW_MSG_AND_REACT);
    buttonEnabled = true;
    log(CLASS_MAIN, Debug, "Done");
  }
}

bool haveToInterrupt() {
  if (Serial.available()) {
    log(CLASS_MAIN, Debug, "Serial pinged: int");
    return true;
  //} else if (buttonInterrupts > 0) {
  //  log(CLASS_MAIN, Debug, "Button pressed: int");
  //  return true;
  } else {
    return false;
  }
}

/*
void initializeServoConfig(const char *tuningFilename, ServoConf **conf) {
  Buffer aux(SERVO_CONF_SERIALIZED_MAX_LENGTH);
  bool succServo0 = readFile(tuningFilename, &aux);
  if (succServo0) {
    aux.replace('\n', 0);
    *conf = new ServoConf(aux.getBuffer());
  } else {
    *conf = new ServoConf();
  }
}

void initializeServoConfigs() {
  initializeServoConfig(SERVO_0_FILENAME, &servo0Conf);
  initializeServoConfig(SERVO_1_FILENAME, &servo1Conf);
}
*/

Buffer *initializeTuningVariable(Buffer **var, const char *filename, int maxLength, const char *defaultContent, bool obfuscate) {
	bool first = false;
  if (*var == NULL) {
  	first = true;
    *var = new Buffer(maxLength);
    bool succAlias = readFile(filename, *var); // preserve the alias
    if (succAlias) {                           // managed to retrieve the alias
      (*var)->replace('\n', 0);                // content already with the alias
    } else if (defaultContent != NULL) {
      (*var)->fill(defaultContent);
    } else {
      abort(filename);
    }
  }
  if (first) {
    if (obfuscate) {
      log(CLASS_MAIN, Info, "Tuning: %s=***", filename);
    } else {
      log(CLASS_MAIN, Info, "Tuning: %s=%s", filename, (*var)->getBuffer());
    }
  }
  return *var;
}

void dumpLogBuffer() {
  if (logBuffer == NULL)
    return;

  Buffer fname(16);
  static int rr = 0;
  rr = (rr + 1) % MAX_ROUND_ROBIN_LOG_FILES;
  fname.fill("%d.log", rr);
  bool suc = writeFile(fname.getBuffer(), logBuffer->getBuffer());
  log(CLASS_MAIN, Warn, "Log: %s %s", fname.getBuffer(), BOOL(suc));
  logBuffer->clear();
}