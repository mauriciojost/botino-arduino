#include <Main.h>

#define CL_MAX_LENGTH 1000
#define CURL_COMMAND_GET "curl --silent -XGET '%s'"
#define CURL_COMMAND_POST "curl --silent -H 'Content-Type: application/json' -XPOST '%s' -d '%s'"

bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries) {
  log(CLASS_MAIN, Debug, "initWifi(%s, %s, %d)", ssid, pass, retries);
  return true;
}

void loopArchitecture() {
  Settings *s = m.getSettings();
  // Handle log level as per settings
  setLogLevel((char)(s->getLogLevel() % 4));

	char str[100];
	gets(str);
	printf("Parsing: '%s'\n", str);
	if (strlen(str) != 0) {
    command(str);
	}
}

void messageFunc(int line, const char *str, int size) {
  printf("\n\n***** LCD (size %d)\n  %s\n*****\n\n", size, str);
}

void logLine(const char *str) {
  printf("LOG: %s\n", str);
}

void arms(int left, int right, int steps) {
  printf("\n\nARMS: %d %d %d\n\n", left, right, steps);
}

int httpGet(const char *url, ParamStream *response) {
  Buffer<CL_MAX_LENGTH> aux;
  aux.fill(CURL_COMMAND_GET, url);
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
  pclose(fp);
  return HTTP_OK; // not quite true, but will work for simple purposes
}

int httpPost(const char *url, const char *body, ParamStream *response) {
  Buffer<CL_MAX_LENGTH> aux;
  aux.fill(CURL_COMMAND_POST, url, body);
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

void ios(char led, bool v) {
  log(CLASS_MAIN, Debug, "Led'%c'->%d", led, (int)v);
}

void lcdImg(char img, uint8_t bitmap[]) {
  log(CLASS_MAIN, Debug, "Img '%c'", img);
}

uint32_t cmillis() {
  return currentMillis;
}

void setupArchitecture() {
  // nothing to be done here
}

void sleepInterruptable(unsigned long cycleBegin, unsigned long periodMsec) {
  log(CLASS_MAIN, Info, "L.Sleep(%lums)...", periodMsec);
}

bool haveToInterrupt() {
  // noting to do here
  return false;
}


int main(int argc, const char *argv[]) {
  setup();
  while (true) {
    loop();
  }
}
