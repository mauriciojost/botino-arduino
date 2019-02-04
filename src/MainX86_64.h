#include <Main.h>

#include <cstdio>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define CL_MAX_LENGTH 5000
#define HTTP_CODE_KEY "HTTP_CODE:"
#define CURL_COMMAND_GET  "/usr/bin/curl --silent -w '" HTTP_CODE_KEY "%%{http_code}' -XGET '%s'"
#define CURL_COMMAND_POST "/usr/bin/curl --silent -w '" HTTP_CODE_KEY "%%{http_code}' -XPOST '%s' -d '%s'"

enum AppMode { Interactive = 0, NonInteractive = 1 };
AppMode appMode = Interactive;

unsigned long millis();

////////////////////////////////////////
// Functions requested for architecture
////////////////////////////////////////

// Callbacks
///////////////////

const char* deviceId() {
	return "PC";
}

void logLine(const char *str) {
  printf("LOG: %s", str);
}

bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries) {
  log(CLASS_MAIN, Debug, "initWifi(%s, %s, %d)", ssid, pass, retries);
  return true;
}

int httpGet(const char *url, ParamStream *response, Table *headers) {
  Buffer aux(CL_MAX_LENGTH);
  int httpCode = HTTP_BAD_REQUEST;
  aux.fill(CURL_COMMAND_GET, url);
  int i = 0;
  while ((i = headers->next(i)) != -1) {
    aux.append(" -H '");
    aux.append(headers->getKey(i));
    aux.append(": ");
    aux.append(headers->getValue(i));
    aux.append("'");
    i++;
  }
  log(CLASS_MAIN, Debug, "GET: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {
    return HTTP_BAD_REQUEST;
  }
  while (fgets(aux.getUnsafeBuffer(), CL_MAX_LENGTH - 1, fp) != NULL) {
    const char* codeStr = aux.since(HTTP_CODE_KEY);
    httpCode = (codeStr!=NULL?atoi(codeStr + strlen(HTTP_CODE_KEY)):HTTP_BAD_REQUEST);
    if (response != NULL) {
      response->fillUntil(aux.getBuffer(), HTTP_CODE_KEY);
      log(CLASS_MAIN, Debug, "-> %s", response->content());
    }
  }
  pclose(fp);
  return httpCode;
}

int httpPost(const char *url, const char *body, ParamStream *response, Table *headers) {
  Buffer aux(CL_MAX_LENGTH);
  int httpCode = HTTP_BAD_REQUEST;
  aux.fill(CURL_COMMAND_POST, url, body);
  int i = 0;
  while ((i = headers->next(i)) != -1) {
    aux.append(" -H '");
    aux.append(headers->getKey(i));
    aux.append(": ");
    aux.append(headers->getValue(i));
    aux.append("'");
    i++;
  }
  log(CLASS_MAIN, Debug, "POST: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {
    log(CLASS_MAIN, Warn, "POST failed");
    return HTTP_BAD_REQUEST;
  }
  while (fgets(aux.getUnsafeBuffer(), CL_MAX_LENGTH - 1, fp) != NULL) {
    const char* codeStr = aux.since(HTTP_CODE_KEY);
    httpCode = (codeStr!=NULL?atoi(codeStr + strlen(HTTP_CODE_KEY)):HTTP_BAD_REQUEST);
    if (response != NULL) {
      response->fillUntil(aux.getBuffer(), HTTP_CODE_KEY);
      log(CLASS_MAIN, Debug, "-> %s", response->content());
    }
  }
  pclose(fp);
  return httpCode;
}

void messageFunc(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str) {
  printf("\n\n***** LCD (size %d)\n  %s\n*****\n\n", size, str);
}

void arms(int left, int right, int steps) {
  printf("\n\nARMS: %d %d %d\n\n", left, right, steps);
}

void ios(char led, IoMode v) {
  log(CLASS_MAIN, Debug, "Led'%c'->%d", led, v);
}

void clearDevice() {
  log(CLASS_MAIN, Debug, "Clear device");
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "Img '%c'", img);
}

bool readFile(const char* fname, Buffer* content) {
	bool success = false;
	char c;
	int i = 0;
	FILE *fp = fopen(fname, "r");
	content->clear();
	if(fp != NULL) {
    while((c = getc(fp)) != EOF){
        content->append(c);
        i++;
    }
    fclose(fp);
    success = true;
	} else {
    log(CLASS_MAIN, Warn, "Could not load file: %s", fname);
    success = false;
	}
	return success;
}

bool writeFile(const char* fname, const char* content) {
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

void infoArchitecture() {}

void testArchitecture() {}

void updateFirmware() {}

// Execution
///////////////////

void sleepInterruptable(time_t cycleBegin, time_t periodSecs) {
  log(CLASS_MAIN, Info, "Sleep(%ds)...", (int)periodSecs);
  sleep(1);
}

BotMode setupArchitecture() {
  log(CLASS_MAIN, Debug, "Setup timing");
  setExternalMillis(millis);
  return RunMode;
}

void runModeArchitecture() {
  if (appMode == Interactive) {
    char str[100];
    printf("Waiting for input: \n   ");
    fgets(str, 100, stdin);
    if (strlen(str) != 0) {
      m->command(str);
    }
  }
}

void configureModeArchitecture() {
  // nothing to be done here
}

void abort(const char* msg) {
  log(CLASS_MAIN, Error, "Abort: %s", msg);
}

////////////////////////////////////////
// Architecture specific functions
////////////////////////////////////////

int main(int argc, const char *argv[]) {
  setup();

  int simulationSteps = 10;

  if (argc == 1) {
    appMode = NonInteractive;
  } else if (argc == 1 + 1) {
    appMode = (AppMode)atoi(argv[1]);
  } else if (argc == 1 + 2) {
    appMode = (AppMode)atoi(argv[1]);
    simulationSteps = atoi(argv[2]);
  } else if (argc != 1 + 2) {
    log(CLASS_MAIN, Error, "2 args max: <starter> [appMode [steps]]");
    return -1;
  }

  for (int i = 0; i < simulationSteps; i++) {
    log(CLASS_MAIN, Debug, "### Step %d/%d", i, simulationSteps);
    loop();
  }
  log(CLASS_MAIN, Debug, "### DONE");
  return 0;
}

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
  // log(CLASS_MAIN, Debug, "Millis: %lu", m);
  return m - boot;
}
