#ifndef PLATFORM_ESP_INC
#define PLATFORM_ESP_INC

#define QUESTION_ANSWER_TIMEOUT_MS 60000

#define RESTORE_WIFI_SSID "assid"
#define RESTORE_WIFI_PASS "apassword"
#define RESTORE_URL "http://main4ino.martinenhome.com/main4ino/prd/firmwares/" PROJECT_ID "/" PLATFORM_ID "/content?version=LATEST"
#define RESTORE_RETRIES 10

#define DELAY_MS_SPI 3
#define ABORT_DELAY_SECS 5
#define HW_STARTUP_DELAY_MSECS 10

#define DEVICE_ALIAS_FILENAME "/alias.tuning"
#define DEVICE_ALIAS_MAX_LENGTH 16

#define DEVICE_PWD_FILENAME "/pass.tuning"
#define DEVICE_PWD_MAX_LENGTH 16

#define SERVO_0_FILENAME "/servo0.tuning"
#define SERVO_1_FILENAME "/servo1.tuning"

#define ABORT_LOG_FILENAME "/abort.log"
#define ABORT_LOG_MAX_LENGTH 64

#define LCD_WIDTH 128
#define LCD_HEIGHT 64

#define LCD_CHAR_WIDTH 6
#define LCD_CHAR_HEIGHT 8

#define SERVO_BASE_STEPS 120
#define SERVO_PERIOD_STEP_MS 2


void restoreSafeFirmware() { // to be invoked as last resource when things go wrong
  initializeWifi(RESTORE_WIFI_SSID, RESTORE_WIFI_PASS, RESTORE_WIFI_SSID, RESTORE_WIFI_PASS, true, RESTORE_RETRIES);
  updateFirmware(RESTORE_URL, STRINGIFY(PROJ_VERSION));
}

void askStringQuestion(const char *question, Buffer *answer) {
  log(CLASS_PLATFORM, User, "Question: %s", question);
  Serial.setTimeout(QUESTION_ANSWER_TIMEOUT_MS);
  Serial.readBytesUntil('\n', answer->getUnsafeBuffer(), answer->getCapacity());
  answer->replace('\n', 0);
  answer->replace('\r', 0);
  log(CLASS_PLATFORM, User, "Answer: '%s'", answer->getBuffer());
}


void initLogBuffer() {
  if (logBuffer == NULL) {
    logBuffer = new Buffer(LOG_BUFFER_MAX_LENGTH);
  }
}

void logLine(const char *str, const char *clz, LogLevel l, bool newline) {
  int ts = (int)((millis()/1000) % 10000);
  Buffer time(8);
  time.fill("%04d|", ts);
  // serial print
#ifdef HEAP_VCC_LOG
  Serial.print("FREE:");
  Serial.print(freeMemory());
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
  bool lcdLogsEnabled = (m==NULL?true:m->getBotinoSettings()->getLcdLogs());
  bool fsLogsEnabled = (m==NULL?true:m->getBotinoSettings()->fsLogsEnabled());
  int fsLogsLength = (m==NULL?DEFAULT_FS_LOGS_LENGTH:m->getBotinoSettings()->getFsLogsLength());

  // lcd print
  if (lcd != NULL && lcdLogsEnabled) { // can be called before LCD initialization
    currentLogLine = NEXT_LOG_LINE_ALGORITHM;
    int line = currentLogLine + 2;
    lcd->setTextWrap(false);
    lcd->fillRect(0, line * LCD_CHAR_HEIGHT, LCD_WIDTH, LCD_CHAR_HEIGHT, BLACK);
    lcd->setTextSize(1);
    lcd->setTextColor(WHITE, !WHITE);
    lcd->setCursor(0, line * LCD_CHAR_HEIGHT);
    lcd->print(str);
    lcd->display();
    delay(DELAY_MS_SPI);
  }
  // local logs (to be sent via network)
  if (fsLogsEnabled) {
    initLogBuffer();
    if (newline) {
      logBuffer->append(time.getBuffer());
    }
    unsigned int s = (unsigned int)(fsLogsLength) + 1;
    char aux2[s];
    strncpy(aux2, str, s);
    aux2[s - 1] = 0;
    aux2[s - 2] = '\n';
    logBuffer->append(aux2);
  }
}

void messageFunc(int x, int y, int color, bool wrap, MsgClearMode clearMode, int size, const char *str) {
  lcd->setTextWrap(wrap);
  lcd->setTextSize(size);
  switch (clearMode) {
    case FullClear:
      lcd->clearDisplay();
      break;
    case LineClear:
      lcd->setTextColor(!color, color);
      lcd->setCursor(x * size * LCD_CHAR_WIDTH, y * size * LCD_CHAR_HEIGHT);
      lcd->print(str);
      break;
    case NoClear:
      break;
  }
  lcd->setTextColor(color, !color);
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


#endif // PLATFORM_ESP_INC

