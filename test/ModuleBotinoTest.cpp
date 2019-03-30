#ifdef UNIT_TEST

// Being tested
#include <ModuleBotino.h>

// Extra libraries needed
#include <main4ino/Misc.h>
#include <string.h>
#include <unity.h>

#define CLASS_MAIN "ModuleBotinoTest"

const char *replyEmptyBody = "{}";

bool wifiConnected;
int pullCount;

void setUp(void) {
  wifiConnected = true;
  pullCount = 1;
}

void tearDown() {}

unsigned long millis() {
  static unsigned long boot = -1;
  struct timespec tms;
  if (clock_gettime(CLOCK_REALTIME, &tms)) {
    log(CLASS_MAIN, Warn, "Couldn't get time");
    return -1;
  }
  unsigned long m = tms.tv_sec * 1000 + tms.tv_nsec / 1000000;
  if (boot == -1) {
    boot = m;
  }
  return m - boot;
}

bool initWifiSimple() {
  return wifiConnected;
}

const char *apiDeviceLogin() {
  return "testdevice";
}

const char *apiDevicePass() {
  return "password";
}

void logLine(const char *str) {
  log(CLASS_MAIN, Debug, "logLine('%s')", str);
}

bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries) {
  log(CLASS_MAIN, Debug, "initWifi('%s', '%s', %d)", ssid, pass, retries);
  return wifiConnected;
}

int httpGet(const char *url, ParamStream *response, Table *headers) {
  char str1[128];
  char str2[128];

  if (sscanf(url, MAIN4INOSERVER_API_HOST_BASE "/api/v1/devices/testdevice/actors/%[a-z]/%[a-z]/last", str1, str2) == 2 && strcmp(str2, "reports") == 0) {
  	if (strcmp(str1, "commands") == 0) {
      log(CLASS_MAIN, Info, "Commands loaded last '%s'", str1);
      response->contentBuffer()->load("{\"cm0\":\"Restored:ack\"}");
  	} else {
      response->contentBuffer()->load(replyEmptyBody);
  	}
    return HTTP_OK;
  } else if (strcmp(MAIN4INOSERVER_API_HOST_BASE "/api/v1/devices/testdevice/targets/count?status=C", url) == 0) {
    response->contentBuffer()->fill("{\"count\":%d}", pullCount);
    return HTTP_OK;
  } else if (sscanf(url, MAIN4INOSERVER_API_HOST_BASE "/api/v1/devices/testdevice/actors/%[a-z]/%[a-z]/summary?consume=true&status=C", str1, str2) ==
             2 && strcmp(str2, "targets") == 0) {
  	if (strcmp(str1, "commands") == 0) {
      log(CLASS_MAIN, Info, "Commands loaded target '%s'", str1);
      response->contentBuffer()->load("{\"cm0\":\"Target:ack\"}");
  	} else {
      response->contentBuffer()->load(replyEmptyBody);
  	}
    return HTTP_OK;
  } else if (sscanf(url, MAIN4INOSERVER_API_HOST_BASE "/api/v1/time?timezone=%s", str1) == 1) {
    response->contentBuffer()->load("{\"formatted\":\"1970-01-01T00:00:01\"}");
    return HTTP_OK;
  } else {
    log(CLASS_MAIN, Debug, "Unknown url '%s'", url);
    return HTTP_BAD_REQUEST;
  }
}

int httpPost(const char *url, const char *body, ParamStream *response, Table *headers) {
  return HTTP_CREATED;
}

void messageFunc(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str) {
  log(CLASS_MAIN, Debug, "\n\n***** LCD (size %d)\n  %s\n*****\n\n", size, str);
}

void arms(int left, int right, int steps) {
  log(CLASS_MAIN, Debug, "arms(%d, %d, %d)", left, right, steps);
}

void ios(char led, IoMode v) {
  log(CLASS_MAIN, Debug, "ios(%c, %d)", led, v);
}

void clearDevice() {
  log(CLASS_MAIN, Debug, "clearDevice()");
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "lcdImg('%c', xxx)", img);
}

bool readFile(const char *f, Buffer *content) {
  Buffer fname(64);
  fname.fill("./test/fs/%s", f);
  log(CLASS_MAIN, Debug, "readFile('%s', xxx)", fname.getBuffer());
  bool success = false;
  char c;
  int i = 0;
  FILE *fp = fopen(fname.getBuffer(), "r");
  content->clear();
  if (fp != NULL) {
    while ((c = getc(fp)) != EOF) {
      content->append(c);
      i++;
    }
    fclose(fp);
    success = true;
  } else {
    log(CLASS_MAIN, Warn, "Could not load file: %s", fname.getBuffer());
    success = false;
  }
  return success;
}

bool writeFile(const char *fname, const char *content) {
  bool success = false;
  FILE *file = fopen(fname, "w+");
  int results = fputs(content, file);
  if (results == EOF) {
    log(CLASS_MAIN, Warn, "Failed to write %s ", fname);
    success = false;
  } else {
    success = true;
  }
  fclose(file);
  return success;
}

void infoArchitecture() {
  log(CLASS_MAIN, Debug, "infoArchitecture()");
}

void testArchitecture() {
  log(CLASS_MAIN, Debug, "testArchitecture()");
}

void updateFirmware(const char* d) {
  log(CLASS_MAIN, Debug, "updateFirmware(%s)", d);
}

bool sleepInterruptable(time_t cycleBegin, time_t periodSecs) {
  log(CLASS_MAIN, Info, "sleepInterruptable(%ds)...", (int)periodSecs);
  return false;
}

BotMode setupArchitecture() {
  log(CLASS_MAIN, Debug, "setupArchitecture()");
  setExternalMillis(millis);
  return RunMode;
}

void runModeArchitecture() {
  log(CLASS_MAIN, Debug, "runModeArchitecture()");
}

CmdExecStatus commandArchitecture(const char *command) {
  log(CLASS_MAIN, Debug, "commandArchitecture('%s')", command);
  return NotFound;
}

void configureModeArchitecture() {
  log(CLASS_MAIN, Debug, "configureModeArchitecture()");
}

void abort(const char *msg) {
  log(CLASS_MAIN, Debug, "abort('%s')", msg);
}

void test_basic_behaviour() {
  ModuleBotino *m = new ModuleBotino();
  TEST_ASSERT_EQUAL(0, (int)m->getBot()->getClock()->currentTime());

  log(CLASS_MAIN, Debug, "### module->setup(...)");
  m->setup(setupArchitecture,
           lcdImg,
           arms,
           messageFunc,
           ios,
           initWifiSimple,
           httpPost,
           httpGet,
           clearDevice,
           readFile,
           writeFile,
           abort,
           sleepInterruptable,
           configureModeArchitecture,
           runModeArchitecture,
           commandArchitecture,
           infoArchitecture,
           updateFirmware,
           testArchitecture,
           apiDeviceLogin,
           apiDevicePass);

  TEST_ASSERT_EQUAL(1, (int)m->getBot()->getClock()->currentTime()); // remote clock sync took place
  TEST_ASSERT_EQUAL_STRING("Restored:ack", m->getCommands()->getCmdNameValue(0)); // loaded previous value

  log(CLASS_MAIN, Debug, "### module->loop()");
  m->getModule()->getPropSync()->getTiming()->setFreq("~1s");
  m->loop();
  TEST_ASSERT_EQUAL_STRING("Target:ack", m->getCommands()->getCmdNameValue(0)); // loaded targets

  log(CLASS_MAIN, Debug, "### module->loop()");
  m->loop();
  TEST_ASSERT_EQUAL_STRING("Target:ack", m->getCommands()->getCmdNameValue(0)); // no change

}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_basic_behaviour);
  return (UNITY_END());
}

#endif // UNIT_TEST
