/**
 * This file aims to be the only HW specific source code file in the whole project.
 * The rest of the classes (and the 100% of their code) should be testeable without need
 * of Arduino specific HW.
 */

#ifndef MAIN_INC
#define MAIN_INC

#define CLASS_MAIN "MA"

#include <Module.h>
#include "Images.h"
#include "main4ino/Misc.h"

#ifndef PROJ_VERSION
#define PROJ_VERSION "1master"
#endif // PROJ_VERSION

#define DELAY_MS_SPI 3
#define FRAG_TO_SLEEP_MS_MAX 2000 // maximum sleeping time for which the module can be unresponsive

#ifndef PERIOD_SEC
#define PERIOD_SEC 60
#endif // PERIOD_SEC

#define PERIOD_MSEC (PERIOD_SEC * 1000)

#define COMMAND_MAX_LENGTH 64

#ifndef DWEET_IO_API_TOKEN
#error "Must provide DWEET_IO_API_TOKEN"
#endif

#define WAIT_BEFORE_HTTP_MS 1500

#define URL_PRINT_MAX_LENGTH 20

// Provided by generic Main
bool initWifiInit();
bool initWifiSteady();
void loop();
void setup();

// To be provided by the Main of a specific architecture
bool initWifi(const char *ssid, const char *pass, bool skipIfConnected, int retries);
void handleSettings();
void handleServices();
void setupArchitecture();
void reactButton();
// Message funcion. Directly connected with user.
void messageFunc(int line, const char *str, int size);
void logLine(const char *str);
void arms(int left, int right, int steps);
int httpGet(const char *url, ParamStream *response);
int httpPost(const char *url, const char *body, ParamStream *response);
void ios(char led, bool v);
void sleepInterruptable(unsigned long cycleBegin);

#endif // MAIN_INC
