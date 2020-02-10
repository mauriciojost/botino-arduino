#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EspSaveCrash.h>
#include <Pinout.h>
#include <Platform.h>
#include <RemoteDebug.h>
#include <SPI.h>
#include <Servo.h>
#include <Wire.h>
#include <utils/Io.h>
#include <utils/ServoConf.h>

#define DELAY_MS_SPI 3
#define ABORT_DELAY_SECS 5
#define HW_STARTUP_DELAY_MSECS 10

#define DEVICE_ALIAS_FILENAME "/alias.tuning"
#define DEVICE_ALIAS_MAX_LENGTH 16

#define DEVICE_PWD_FILENAME "/pass.tuning"
#define DEVICE_PWD_MAX_LENGTH 16

#define SERVO_0_FILENAME "/servo0.tuning"
#define SERVO_1_FILENAME "/servo1.tuning"
#define SLEEP_PERIOD_UPON_BOOT_SEC 2

#define LCD_WIDTH 128
#define LCD_HEIGHT 64

#define LCD_CHAR_WIDTH 6
#define LCD_CHAR_HEIGHT 8

#define SERVO_BASE_STEPS 120
#define SERVO_PERIOD_STEP_MS 2

#define NEXT_LOG_LINE_ALGORITHM ((currentLogLine + 1) % 6)

#define LOG_BUFFER_MAX_LENGTH 1024

#define USER_LCD_FONT_SIZE 2

#define VCC_FLOAT ((float)ESP.getVcc() / 1024)

#define ONLY_SHOW_MSG true
#define SHOW_MSG_AND_REACT false

extern "C" {
#include "user_interface.h"
}

#define HELP_COMMAND_ARCH_CLI                                                                                                              \
  "\n  ESP8266 HELP"                                                                                                                       \
  "\n  init              : initialize essential settings (wifi connection, logins, etc.)"                                                  \
  "\n  servo ...         : tune the servo <s> (r|l) and make a test round "                                                                \
  "\n  rm ...            : remove file in FS "                                                                                             \
  "\n  ls                : list files present in FS "                                                                                      \
  "\n  reset             : reset the device"                                                                                               \
  "\n  freq ...          : set clock frequency in MHz (80 or 160 available only, 160 faster but more power consumption)"                   \
  "\n  lightsleep ...    : light sleep N provided seconds"                                                                                 \
  "\n  clearstack        : clear stack trace "                                                                                             \
  "\n"

#define BUTTON_IS_PRESSED ((bool)digitalRead(BUTTON0_PIN))

volatile bool buttonEnabled = true;
volatile unsigned char buttonInterrupts = 0;

#ifdef TELNET_ENABLED
RemoteDebug telnet;
#endif // TELNET_ENABLED
Servo servoLeft;
Servo servoRight;
Adafruit_SSD1306 *lcd = NULL;
Buffer *apiDeviceId = NULL;
Buffer *apiDevicePwd = NULL;
ServoConf *servo0Conf = NULL;
ServoConf *servo1Conf = NULL;
int currentLogLine = 0;
Buffer *cmdBuffer = NULL;
Buffer *cmdLast = NULL;
EspSaveCrash espSaveCrash;

#define LED_INT_TOGGLE ios('y', IoToggle);
#define LED_INT_ON ios('y', IoOn);
#define LED_ALIVE_TOGGLE ios('r', IoToggle);

ADC_MODE(ADC_VCC);

void bitmapToLcd(uint8_t bitmap[]);
void reactCommandCustom();
void buttonPressed();
void heartbeat();
void debugHandle();
bool haveToInterrupt();
void handleInterrupt();
void initializeServoConfigs();

#include <primitives/BoardESP8266.h>

////////////////////////////////////////
// Functions requested for architecture
////////////////////////////////////////

// Callbacks
///////////////////

bool initWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfAlreadyConnected, int retries) {
  return initializeWifi(ssid, pass, ssidb, passb, skipIfAlreadyConnected, retries);
}

const char *apiDeviceLogin() {
  return initializeTuningVariable(&apiDeviceId, DEVICE_ALIAS_FILENAME, DEVICE_ALIAS_MAX_LENGTH, NULL, false)->getBuffer();
}

const char *apiDevicePass() {
  return initializeTuningVariable(&apiDevicePwd, DEVICE_PWD_FILENAME, DEVICE_PWD_MAX_LENGTH, NULL, true)->getBuffer();
}

void logLine(const char *str, const char *clz, LogLevel l) {
  int ts = (int)((millis()/1000) % 10000);
  Buffer aux(8);
  aux.fill("%04d|", ts);
  // serial print
#ifdef HEAP_VCC_LOG
  Serial.print("HEA:");
  Serial.print(ESP.getFreeHeap());
  Serial.print("|");
  Serial.print("VCC:");
  Serial.print(VCC_FLOAT);
  Serial.print("|");
#endif // HEAP_VCC_LOG
  Serial.print(str);
  // telnet print
#ifdef TELNET_ENABLED
  if (telnet.isActive()) {
    for (unsigned int i = 0; i < strlen(str); i++) {
      telnet.write(str[i]);
    }
  }
#endif // TELNET_ENABLED
  if (m == NULL) {
    return;
  }
  // lcd print
  if (lcd != NULL && m->getBotinoSettings()->getLcdLogs()) { // can be called before LCD initialization
    currentLogLine = NEXT_LOG_LINE_ALGORITHM;
    int line = currentLogLine + 2;
    lcd->setTextWrap(false);
    lcd->fillRect(0, line * LCD_CHAR_HEIGHT, LCD_WIDTH, LCD_CHAR_HEIGHT, BLACK);
    lcd->setTextSize(1);
    lcd->setTextColor(WHITE);
    lcd->setCursor(0, line * LCD_CHAR_HEIGHT);
    lcd->print(str);
    lcd->display();
    delay(DELAY_MS_SPI);
  }
  // local logs (to be sent via network)
  if (m->getBotinoSettings()->fsLogsEnabled()) {
    if (logBuffer == NULL) {
      logBuffer = new Buffer(LOG_BUFFER_MAX_LENGTH);
    }
    logBuffer->append(aux.getBuffer());
    logBuffer->append(str);
  }
}

void messageFunc(int x, int y, int color, bool wrap, MsgClearMode clearMode, int size, const char *str) {
  switch (clearMode) {
    case FullClear:
      lcd->clearDisplay();
      break;
    case LineClear:
      lcd->fillRect(x * size * LCD_CHAR_WIDTH, y * size * LCD_CHAR_HEIGHT, LCD_WIDTH, size * LCD_CHAR_HEIGHT, !color);
      wrap = false;
      break;
    case NoClear:
      break;
  }
  lcd->setTextWrap(wrap);
  lcd->setTextSize(size);
  lcd->setTextColor(color);
  lcd->setCursor(x * size * LCD_CHAR_WIDTH, y * size * LCD_CHAR_HEIGHT);
  lcd->print(str);
  lcd->display();
  log(CLASS_PLATFORM, Debug, "Msg: (%d,%d)'%s'", x, y, str);
  delay(DELAY_MS_SPI);
}

void arms(int left, int right, int periodFactor) {
  static int lastDegL = -1;
  static int lastDegR = -1;

  int steps = periodFactor * SERVO_BASE_STEPS;

  log(CLASS_PLATFORM, Debug, "Arms>%d&%d", left, right);

  int targetDegL = servo0Conf->getTargetDegreesFromPosition(left);
  servoLeft.attach(SERVO0_PIN);

  int targetDegR = servo1Conf->getTargetDegreesFromPosition(right);
  servoRight.attach(SERVO1_PIN);

  // leave as target if first time
  lastDegL = (lastDegL == -1 ? targetDegL : lastDegL);
  lastDegR = (lastDegR == -1 ? targetDegR : lastDegR);

  log(CLASS_PLATFORM, Debug, "Sv.Ldeg%d>deg%d", lastDegL, targetDegL);
  log(CLASS_PLATFORM, Debug, "Sv.Rdeg%d>deg%d", lastDegR, targetDegR);
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
    log(CLASS_PLATFORM, Warn, "Unknown pin: %c", pin);
    return;
  }
  if (value == IoOn) {
    digitalWrite(pin, LOW);
  } else if (value == IoOff) {
    digitalWrite(pin, HIGH);
  } else if (value == IoToggle) {
    digitalWrite(pin, !digitalRead(pin));
  } else {
    log(CLASS_PLATFORM, Warn, "Unknown IO request: %c<-%d", pin, value);
  }
}

void clearDevice() {
  log(CLASS_PLATFORM, User, "   rm %s", DEVICE_ALIAS_FILENAME);
  log(CLASS_PLATFORM, User, "   rm %s", DEVICE_PWD_FILENAME);
  log(CLASS_PLATFORM, User, "   ls");
  log(CLASS_PLATFORM, User, "   <remove all .properties>");
  espSaveCrash.clear();
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_PLATFORM, Debug, "Img '%c'", img);
  switch (img) {
    case '_': // dim
      log(CLASS_PLATFORM, Debug, "Dim face");
      lcd->dim(true);
      break;
    case '-': // bright
      log(CLASS_PLATFORM, Debug, "Bright face");
      lcd->dim(false);
      break;
    case 'w': // white
      log(CLASS_PLATFORM, Debug, "White face");
      lcd->invertDisplay(true);
      break;
    case 'b': // black
      log(CLASS_PLATFORM, Debug, "Black face");
      lcd->invertDisplay(false);
      break;
    case 'l': // clear
      log(CLASS_PLATFORM, Debug, "Clear face");
      lcd->invertDisplay(false);
      lcd->clearDisplay();
      break;
    case 'c': // custom
      log(CLASS_PLATFORM, Debug, "Custom face", img);
      if (bitmap != NULL) {
        logHex(CLASS_PLATFORM, Debug, bitmap, IMG_SIZE_BYTES);
        bitmapToLcd(bitmap); // custom
      }
      break;
    default:
      log(CLASS_PLATFORM, Debug, "Face?: %c", img);
      break;
  }
  lcd->display();
  delay(DELAY_MS_SPI);
}

void infoArchitecture() {

  m->getNotifier()->message(0,
                            1,
                            "ID:%s\nV:%s\nCrashes:%d\nIP: %s\nMemory:%lu\nUptime:%luh\nVcc: %0.2f",
                            apiDeviceLogin(),
                            STRINGIFY(PROJ_VERSION),
                            espSaveCrash.count(),
                            WiFi.localIP().toString().c_str(),
                            ESP.getFreeHeap(),
                            (millis() / 1000) / 3600,
                            VCC_FLOAT);
}

void testArchitecture() {}

// Execution
///////////////////

BotMode setupArchitecture() {

  // Let HW startup
  delay(HW_STARTUP_DELAY_MSECS);

  // Intialize the logging framework
  Serial.begin(115200);     // Initialize serial port
  Serial.setTimeout(10000); // Timeout for read
  setupLog(logLine);
  setLogLevel(Info);
  log(CLASS_PLATFORM, Info, "Log initialized");

  log(CLASS_PLATFORM, Debug, "Setup cmds");
  cmdBuffer = new Buffer(COMMAND_MAX_LENGTH);
  cmdLast = new Buffer(COMMAND_MAX_LENGTH);
  log(CLASS_PLATFORM, Debug, "Setup timing");
  setExternalMillis(millis);

  log(CLASS_PLATFORM, Debug, "Setup SPIFFS");

  log(CLASS_PLATFORM, Debug, "Setup pins");
  pinMode(LEDR_PIN, OUTPUT);
  pinMode(LEDW_PIN, OUTPUT);
  pinMode(LEDY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(SERVO0_PIN, OUTPUT);
  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(BUTTON0_PIN, INPUT);

  log(CLASS_PLATFORM, Debug, "Setup LCD");
  lcd = new Adafruit_SSD1306(-1);
  lcd->begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize LCD
  delay(DELAY_MS_SPI);
  heartbeat();

  log(CLASS_PLATFORM, Debug, "Setup wdt");
  ESP.wdtEnable(1); // argument not used

  log(CLASS_PLATFORM, Debug, "Setup wifi");
  WiFi.persistent(false);
  wifi_set_sleep_type(NONE_SLEEP_T); // sleep disabled mode
  WiFi.hostname(apiDeviceLogin());
  heartbeat();

  log(CLASS_PLATFORM, Debug, "Setup http");
  httpClient.setTimeout(HTTP_TIMEOUT_MS);
  httpClient.setUserAgent(apiDeviceLogin());
  heartbeat();

  log(CLASS_PLATFORM, Debug, "Setup random");
  randomSeed(analogRead(0) * 256 + analogRead(0));
  heartbeat();

  log(CLASS_PLATFORM, Debug, "Setup interrupts");
  attachInterrupt(digitalPinToInterrupt(BUTTON0_PIN), buttonPressed, RISING);
  heartbeat();

  log(CLASS_PLATFORM, Debug, "Setup commands");
#ifdef TELNET_ENABLED
  telnet.setCallBackProjectCmds(reactCommandCustom);
  String helpCli("Type 'help' for help");
  telnet.setHelpProjectsCmds(helpCli);
#endif // TELNET_ENABLED
  heartbeat();

  log(CLASS_PLATFORM, Debug, "Setup IO/lcd");
  ios('*', IoOff);
  lcdImg('l', NULL);
  heartbeat();

  log(CLASS_PLATFORM, Debug, "Setup servos");
  initializeServoConfigs();

  log(CLASS_PLATFORM, Debug, "Clean up crashes");
  if (espSaveCrash.count() > 5) {
    log(CLASS_PLATFORM, Warn, "Too many Stack-trcs / clearing (!!!)");
    espSaveCrash.clear();
  } else if (espSaveCrash.count() > 0) {
    log(CLASS_PLATFORM, Warn, "Stack-trcs (!!!)");
    espSaveCrash.print();
  }

  log(CLASS_PLATFORM, Debug, "Letting user interrupt...");
  bool i = sleepInterruptable(now(), SLEEP_PERIOD_UPON_BOOT_SEC);
  if (i) {
    log(CLASS_PLATFORM, Info, "Arch. setup OK => configure mode");
    return ConfigureMode;
  } else {
    log(CLASS_PLATFORM, Info, "Arch. setup OK => run mode");
    return RunMode;
  }
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
    log(CLASS_MODULE, Info, "Moves: %d/%d %s", d, testRange, (pressed ? "<= IN RANGE" : ""));
    min = ((d < min) && pressed ? d : min);
    max = ((d > max) && pressed ? d : max);
    servo->write(d);
    delay(SERVO_PERIOD_STEP_MS * SERVO_BASE_STEPS);
  }

  bool dwn = askBoolQuestion("Is the arm\ndown now?");
  m->getNotifier()->message(0, USER_LCD_FONT_SIZE, "Arm now:\n%s", (dwn ? "DOWN" : "UP"));
  delay(USER_DELAY_MS);
  m->getNotifier()->message(0, 2, "Setup\n%s\ndone!", name);
  delay(USER_DELAY_MS);

  servoConf->setBase(min);
  servoConf->setRange(max - min);
  servoConf->setInvert(dwn);

  servo->detach();
}

CmdExecStatus commandArchitecture(const char *c) {
  if (strcmp("servo", c) == 0) {
    char servo = strtok(NULL, " ")[0];
    Buffer serialized(16);
    if (servo == 'r' || servo == 'R') {
      arms(2, 2, 1);
      arms(2, 7, 1);
      arms(2, 2, 1);
      tuneServo("right servo", SERVO1_PIN, &servoRight, servo1Conf);
      servo1Conf->serialize(&serialized); // right servo1
      writeFile(SERVO_1_FILENAME, serialized.getBuffer());
      log(CLASS_PLATFORM, User, "Stored tuning right servo");
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
      log(CLASS_PLATFORM, User, "Stored tuning left servo");
      arms(0, 0, 1);
      arms(9, 0, 1);
      arms(0, 0, 1);
      return Executed;
    } else {
      log(CLASS_PLATFORM, User, "Invalid servo (l|r)");
      return InvalidArgs;
    }
  } else if (strcmp("init", c) == 0) {
    logRaw(CLASS_PLATFORM, User, "-> Initialize");
    logRaw(CLASS_PLATFORM, User, "Execute:");
    logRaw(CLASS_PLATFORM, User, "   ls");
    log(CLASS_PLATFORM, User, "   save %s <alias>", DEVICE_ALIAS_FILENAME);
    log(CLASS_PLATFORM, User, "   save %s <pwd>", DEVICE_PWD_FILENAME);
    logRaw(CLASS_PLATFORM, User, "   servo l");
    logRaw(CLASS_PLATFORM, User, "   servo r");
    logRaw(CLASS_PLATFORM, User, "   wifissid <ssid>");
    logRaw(CLASS_PLATFORM, User, "   wifipass <password>");
    logRaw(CLASS_PLATFORM, User, "   wifissidb <ssidb>");
    logRaw(CLASS_PLATFORM, User, "   wifipassb <passwordb>");
    logRaw(CLASS_PLATFORM, User, "   ifttttoken <token>");
    logRaw(CLASS_PLATFORM, User, "   (setup of power consumption settings architecture specific if any)");
    logRaw(CLASS_PLATFORM, User, "   store");
    logRaw(CLASS_PLATFORM, User, "   ls");
    return Executed;
  } else if (strcmp("ls", c) == 0) {
    SPIFFS.begin();
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      log(CLASS_PLATFORM, User, "- %s (%d bytes)", dir.fileName().c_str(), (int)dir.fileSize());
    }
    SPIFFS.end();
    return Executed;
  } else if (strcmp("rm", c) == 0) {
    const char *f = strtok(NULL, " ");
    SPIFFS.begin();
    bool succ = SPIFFS.remove(f);
    log(CLASS_PLATFORM, User, "### File '%s' %s removed", f, (succ ? "" : "NOT"));
    SPIFFS.end();
    return Executed;
  } else if (strcmp("reset", c) == 0) {
    ESP.restart(); // it is normal that it fails if invoked the first time after firmware is written
    return Executed;
  } else if (strcmp("freq", c) == 0) {
    uint8 fmhz = (uint8)atoi(strtok(NULL, " "));
    bool succ = system_update_cpu_freq(fmhz);
    log(CLASS_PLATFORM, User, "Freq updated: %dMHz (succ %s)", (int)fmhz, BOOL(succ));
    return Executed;
  } else if (strcmp("lightsleep", c) == 0) {
    int s = atoi(strtok(NULL, " "));
    return (lightSleepInterruptable(now(), s, 1000, haveToInterrupt, heartbeat) ? ExecutedInterrupt
                                                                                                                    : Executed);
  } else if (strcmp("clearstack", c) == 0) {
    espSaveCrash.clear();
    return Executed;
  } else if (strcmp("help", c) == 0 || strcmp("?", c) == 0) {
    logRaw(CLASS_PLATFORM, User, HELP_COMMAND_ARCH_CLI);
    return Executed;
  } else {
    return NotFound;
  }
}

void configureModeArchitecture() {
  handleInterrupt();
  debugHandle();
#ifdef TELNET_ENABLED
  if (m->getBot()->getClock()->currentTime() % 60 == 0) { // every minute
    m->getNotifier()->message(0, 2, "telnet %s", WiFi.localIP().toString().c_str());
  }
#endif // TELNET_ENABLED
}

void abort(const char *msg) {
  log(CLASS_PLATFORM, Error, "Abort: %s", msg);
  m->getNotifier()->message(0, 2, "Abort: %s", msg);
  bool interrupt = sleepInterruptable(now(), ABORT_DELAY_SECS);
  if (interrupt) {
    log(CLASS_PLATFORM, Debug, "Abort sleep interrupted");
  } else {
    log(CLASS_PLATFORM, Warn, "Will light sleep and restart upon abort...");
    bool i = sleepInterruptable(now(), 120L);
    if (!i) {
      ESP.restart();
    } else {
      log(CLASS_PLATFORM, Warn, "Restart skipped because of interrupt.");
      log(CLASS_PLATFORM, Warn, "System ready for exploration.");
    }
  }
}

////////////////////////////////////////
// Architecture specific functions
////////////////////////////////////////

void debugHandle() {
  if (!m->getModuleSettings()->getDebug()) {
    return;
  }
  static bool firstTime = true;
  Serial.setDebugOutput(getLogLevel() == Debug && m->getModuleSettings()->getDebug()); // deep HW logs
  if (firstTime) {
    log(CLASS_PLATFORM, Debug, "Initialize debuggers...");
#ifdef TELNET_ENABLED
    telnet.begin(apiDeviceLogin()); // Intialize the remote logging framework
#endif // TELNET_ENABLED
#ifdef OTA_ENABLED
    ArduinoOTA.begin();             // Intialize OTA
#endif // OTA_ENABLED
    firstTime = false;
  }

  m->getBotinoSettings()->getStatus()->fill("vcc:%0.2f,freeheap:%d", VCC_FLOAT, ESP.getFreeHeap());
  m->getBotinoSettings()->getMetadata()->changed();

#ifdef TELNET_ENABLED
  telnet.handle();     // Handle telnet log server and commands
#endif // TELNET_ENABLED
#ifdef OTA_ENABLED
  ArduinoOTA.handle(); // Handle on the air firmware load
#endif // OTA_ENABLED
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
  if (m == NULL) {
    return;
  }
#ifdef TELNET_ENABLED
  m->command(telnet.getLastCommand().c_str());
#endif // TELNET_ENABLED
}

void heartbeat() {

#ifdef VISUAL_HEARTBEAT
  if (lcd == NULL) {
    return;
  }
  int x = ((LCD_WIDTH / LCD_CHAR_WIDTH) - 1) * LCD_CHAR_WIDTH;    // right
  int y = ((LCD_HEIGHT / LCD_CHAR_HEIGHT) - 1) * LCD_CHAR_HEIGHT; // bottom
  char c = 0x03;                                                  // heart
  int size = 1;                                                   // small
#endif // VISUAL_HEARTBEAT

  LED_ALIVE_TOGGLE
#ifdef VISUAL_HEARTBEAT
  lcd->drawChar(x, y, c, 1, 0, size);
  lcd->display();
#endif // VISUAL_HEARTBEAT
  delay(4);
#ifdef VISUAL_HEARTBEAT
  lcd->drawChar(x, y, c, 0, 0, size);
  lcd->display();
#endif // VISUAL_HEARTBEAT
  LED_ALIVE_TOGGLE
}

void handleInterrupt() {
  if (Serial.available()) {
    // Handle serial commands
    uint8_t c;

    while (true) {
      int inLoop = 0;
      size_t n = Serial.readBytes(&c, 1);

      if (c == 0x08 && n == 1) { // backspace
        log(CLASS_PLATFORM, Debug, "Backspace");
        if (cmdBuffer->getLength() > 0) {
          cmdBuffer->getUnsafeBuffer()[cmdBuffer->getLength() - 1] = 0;
        }
      } else if (c == 0x1b && n == 1) { // up/down
        log(CLASS_PLATFORM, Debug, "Up/down");
        cmdBuffer->load(cmdLast->getBuffer());
      } else if ((c == '\n' || c == '\r') && n == 1) { // if enter is pressed...
        log(CLASS_PLATFORM, Debug, "Enter");
        if (cmdBuffer->getLength() > 0) {
          CmdExecStatus execStatus = m->command(cmdBuffer->getBuffer());
          bool interrupt = (execStatus == ExecutedInterrupt);
          log(CLASS_PLATFORM, Debug, "Interrupt: %d", interrupt);
          log(CLASS_PLATFORM, Debug, "Cmd status: %s", CMD_EXEC_STATUS(execStatus));
          log(CLASS_PLATFORM, User, "('%s' => %s)", cmdBuffer->getBuffer(), CMD_EXEC_STATUS(execStatus));
          cmdLast->load(cmdBuffer->getBuffer());
          cmdBuffer->clear();
        }
        break;
      } else if (n == 1) {
        cmdBuffer->append(c);
      }
      // echo
      log(CLASS_PLATFORM, User, "> %s (%d)", cmdBuffer->getBuffer(), (int)c);
      while (!Serial.available() && inLoop < USER_INTERACTION_LOOPS_MAX) {
        inLoop++;
        delay(100);
      }
      if (inLoop >= USER_INTERACTION_LOOPS_MAX) {
        log(CLASS_PLATFORM, User, "> (timeout)");
        break;
      }
    }
    log(CLASS_PLATFORM, Debug, "Done with interrupt");

  } else if (buttonInterrupts > 0) {
    buttonEnabled = false;
    buttonInterrupts = 0; // to avoid interrupting whatever is called below
    int holds = -1;
    do {
      holds++;
      log(CLASS_PLATFORM, Debug, "%d", holds);
      LED_INT_TOGGLE;
      m->sequentialCommand(holds, ONLY_SHOW_MSG);
      LED_INT_TOGGLE;
      delay(m->getModuleSettings()->miniPeriodMsec());
    } while (BUTTON_IS_PRESSED);
    m->sequentialCommand(holds, SHOW_MSG_AND_REACT);
    buttonEnabled = true;
    log(CLASS_PLATFORM, Debug, "Done");
  }
}

bool haveToInterrupt() {
  if (Serial.available()) {
    log(CLASS_PLATFORM, Debug, "Serial pinged: int");
    return true;
  } else if (buttonInterrupts > 0) {
    log(CLASS_PLATFORM, Debug, "Button pressed: int");
    return true;
  } else {
    return false;
  }
}

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
