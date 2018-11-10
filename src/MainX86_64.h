#include <Main.h>

#include <cstdio>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define CL_MAX_LENGTH 1000
#define CURL_COMMAND_GET "curl --silent -XGET '%s'"
#define CURL_COMMAND_POST "curl --silent -H 'Content-Type: application/json' -XPOST '%s' -d '%s'"

enum AppMode { Interactive = 0, NonInteractive = 1 };
AppMode appMode = Interactive;

unsigned long millis();

////////////////////////////////////////
// Functions requested for architecture
////////////////////////////////////////

// Callbacks
///////////////////

void logLine(const char *str) {
  printf("LOG: %s", str);
}

bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries) {
  log(CLASS_MAIN, Debug, "initWifi(%s, %s, %d)", ssid, pass, retries);
  return true;
}

int httpGet(const char *url, ParamStream *response, Table *headers) {
  Buffer aux(CL_MAX_LENGTH);
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
    if (response != NULL) {
      response->fill(aux.getBuffer());
      log(CLASS_MAIN, Debug, "-> %s", response->content());
    }
  }
  if ((pclose(fp) / 256) == 0) { // not quite true, but will work for simple purposes
    return HTTP_OK;
  } else {
    return HTTP_BAD_REQUEST;
  }
}

int httpPost(const char *url, const char *body, ParamStream *response, Table *headers) {
  Buffer aux(CL_MAX_LENGTH);
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
    return HTTP_BAD_REQUEST;
  }
  while (fgets(aux.getUnsafeBuffer(), CL_MAX_LENGTH - 1, fp) != NULL) {
    if (response != NULL) {
      response->fill(aux.getBuffer());
      log(CLASS_MAIN, Debug, "-> %s", response->content());
    }
  }
  pclose(fp);
  return HTTP_OK; // not quite true, but will work for simple purposes
}

void messageFunc(int x, int y, int color, bool wrap, bool clear, int size, const char *str) {
  printf("\n\n***** LCD (size %d)\n  %s\n*****\n\n", size, str);
}

void arms(int left, int right, int steps) {
  printf("\n\nARMS: %d %d %d\n\n", left, right, steps);
}

void ios(char led, bool v) {
  log(CLASS_MAIN, Debug, "Led'%c'->%d", led, (int)v);
}

void clearDevice() {
  log(CLASS_MAIN, Debug, "Clear device");
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "Img '%c'", img);
}

void readFile(const char* fname, Buffer* content) {
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
	} else {
    log(CLASS_MAIN, Warn, "Could not load file: %s", fname);
	}
}

void writeFile(const char* fname, const char* content) {
	FILE *file = fopen(fname, "w+");
	int results = fputs(content, file);
	if (results == EOF) {
    log(CLASS_MAIN, Warn, "Failed to write %s ", fname);
	}
	fclose(file);
}

// Execution
///////////////////

void sleepInterruptable(unsigned long cycleBegin, unsigned long periodMsec) {
  log(CLASS_MAIN, Info, "L.Sleep(%lums)...", periodMsec);
  sleep(1);
}

bool haveToInterrupt() {
  // noting to do here
  return false;
}

void setupArchitecture() {
  setExternalMillis(millis);
}

void runModeArchitecture() {
  if (appMode == Interactive) {
    char str[100];
    printf("Waiting for input: \n   ");
    fgets(str, 100, stdin);
    if (strlen(str) != 0) {
      m.command(str);
    }
  }
}

void configureModeArchitecture() {
  // nothing to be done here
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
