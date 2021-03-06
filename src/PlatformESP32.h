#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32_Servo.h>
#include <PinoutESP32.h>
#include <Platform.h>
#include <RemoteDebug.h>
#include <SPI.h>
#include <Wire.h>
#include <primitives/BoardESP32.h>
#include <utils/Io.h>
#include <utils/ServoConf.h>
#ifndef TELNET_HANDLE_DELAY_MS
#define TELNET_HANDLE_DELAY_MS 240000 // 4 minutes
#endif                                // TELNET_HANDLE_DELAY_MS

#define FORMAT_SPIFFS_IF_FAILED true

#define NEXT_LOG_LINE_ALGORITHM ((currentLogLine + 1) % 6)

#define LOG_BUFFER_MAX_LENGTH 1024

#define USER_LCD_FONT_SIZE 2

//#define VCC_FLOAT ((float)ESP.getVcc() / 1024)

#define ONLY_SHOW_MSG true
#define SHOW_MSG_AND_REACT false


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

#define LED_INT_TOGGLE ios('y', IoToggle);
#define LED_INT_ON ios('y', IoOn);
#define LED_ALIVE_TOGGLE ios('r', IoToggle);

// ADC_MODE(ADC_VCC);

void bitmapToLcd(uint8_t bitmap[]);
void reactCommandCustom();
void buttonPressed();
void heartbeat();
void debugHandle();
bool haveToInterrupt();
void handleInterruptCustom();
void initializeServoConfigs();

#include <PlatformESP.h>

////////////////////////////////////////
// Functions requested for architecture
////////////////////////////////////////

// Callbacks
///////////////////

bool buttonIsPressed() {
  return BUTTON_IS_PRESSED;
}

const char *apiDeviceLogin() {
  return initializeTuningVariable(&apiDeviceId, DEVICE_ALIAS_FILENAME, DEVICE_ALIAS_MAX_LENGTH, NULL, false)->getBuffer();
}

const char *apiDevicePass() {
  return initializeTuningVariable(&apiDevicePwd, DEVICE_PWD_FILENAME, DEVICE_PWD_MAX_LENGTH, NULL, true)->getBuffer();
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
#ifdef ESP32_LEDR_INVERT
      value = invert(value);
#endif // ESP32_LEDR_INVERT
      break;
    case 'w':
      pin = LEDW_PIN;
#ifdef ESP32_LEDW_INVERT
      value = invert(value);
#endif // ESP32_LEDW_INVERT
      break;
    case 'y':
      pin = LEDY_PIN;
#ifdef ESP32_LEDY_INVERT
      value = invert(value);
#endif // ESP32_LEDY_INVERT
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

void testArchitecture() {}

// Execution
///////////////////

void setupArchitecture() {

  // Let HW startup
  delay(HW_STARTUP_DELAY_MSECS);

  // Intialize the logging framework
  Serial.begin(115200);     // Initialize serial port
  Serial.setTimeout(10000); // Timeout for read
  setupLog(logLine);

  log(CLASS_PLATFORM, User, "BOOT");
  log(CLASS_PLATFORM, User, "%s", STRINGIFY(PROJ_VERSION));

  log(CLASS_PLATFORM, Debug, "Setup cmds");
  cmdBuffer = new Buffer(COMMAND_MAX_LENGTH);
  cmdLast = new Buffer(COMMAND_MAX_LENGTH);

  log(CLASS_PLATFORM, Debug, "Setup timing");
  setExternalMillis(millis);

  log(CLASS_PLATFORM, Debug, "Setup SPIFFS");
  SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);

  log(CLASS_PLATFORM, Debug, "Setup pins");
  pinMode(LEDR_PIN, OUTPUT);
  pinMode(LEDW_PIN, OUTPUT);
  pinMode(LEDY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(SERVO0_PIN, OUTPUT);
  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(BUTTON0_PIN, INPUT);

  // log(CLASS_PLATFORM, Debug, "Setup wdt");
  // ESP.wdtEnable(1); // argument not used

  log(CLASS_PLATFORM, Debug, "Setup LCD");
  lcd = new Adafruit_SSD1306(-1);
  lcd->begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize LCD
  delay(DELAY_MS_SPI);
  heartbeat();

  log(CLASS_PLATFORM, Debug, "Setup wifi");
  WiFi.persistent(false);
  WiFi.setHostname(apiDeviceLogin());
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
  heartbeat();
  Buffer fcontent(ABORT_LOG_MAX_LENGTH);
  bool abrt = readFile(ABORT_LOG_FILENAME, &fcontent);
  if (abrt) {
    log(CLASS_PLATFORM, Warn, "Abort: %s", fcontent.getBuffer());

    SPIFFS.remove(ABORT_LOG_FILENAME);

  } else {
    log(CLASS_PLATFORM, Debug, "No abort");
  }
}

void runModeArchitecture() {
  handleInterruptCustom();
  if (m->getModuleSettings()->getDebugFlag('d')) {
    debugHandle();
  }
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

CmdExecStatus commandArchitecture(Cmd *c) {
  if (c->matches("servo", "initialize the servo", 1, "r|l")) {
    char servo = (char)c->getArgIntBE(0);
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
  } else if (c->matches("init", "initialize all device", 0)) {
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
  } else {
    return NotFound;
  }
}

void configureModeArchitecture() {
  handleInterruptCustom();
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

  Buffer fcontent(ABORT_LOG_MAX_LENGTH);
  fcontent.fill("time=%ld msg=%s", now(), msg);
  writeFile(ABORT_LOG_FILENAME, fcontent.getBuffer());

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
  if (!m->getModuleSettings()->getDebugFlag('d')) {
    return;
  }
  static bool firstTime = true;
  Serial.setDebugOutput(m->getModuleSettings()->getDebugFlag('d')); // deep HW logs
  if (firstTime) {
    log(CLASS_PLATFORM, Debug, "Initialize debuggers...");
#ifdef TELNET_ENABLED
    telnet.begin(apiDeviceLogin()); // Intialize the remote logging framework
#endif                              // TELNET_ENABLED
#ifdef OTA_ENABLED
    ArduinoOTA.begin(); // Intialize OTA
#endif                  // OTA_ENABLED
    firstTime = false;
  }

  m->getBotinoSettings()->getStatus()->fill("freeheap:%d/%d block:%d-%d uptimeh:%d",
                                            (int)ESP.getFreeHeap(),
                                            (int)ESP.getHeapSize(),
                                            (int)ESP.getMaxAllocHeap(),
                                            (int)ESP.getMaxAllocPsram(),
                                            millis() / (1000 * 3600));
  m->getBotinoSettings()->getMetadata()->changed();

#ifdef TELNET_ENABLED
  log(CLASS_PLATFORM, User, "telnet?");
  for (int i = 0; i < TELNET_HANDLE_DELAY_MS / 1000; i++) {
    telnet.handle(); // Handle telnet log server and commands
    delay(1000);
  }
#endif // TELNET_ENABLED
#ifdef OTA_ENABLED
  ArduinoOTA.handle(); // Handle on the air firmware load
#endif                 // OTA_ENABLED
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
#endif                                                            // VISUAL_HEARTBEAT

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

void handleInterruptCustom() {
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
      } else if (c == '\n' && n == 1) { // if enter is pressed...
        log(CLASS_PLATFORM, Debug, "Enter");
        cmdBuffer->replace('\n', 0);
        cmdBuffer->replace('\r', 0);
        if (cmdBuffer->getLength() > 0) {
          Cmd cmd(cmdBuffer->getBuffer());
          CmdExecStatus execStatus = m->getBot()->command(&cmd);
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
