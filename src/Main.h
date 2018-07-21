/**
 * This file aims to be the only HW specific source code file in the whole project.
 * The rest of the classes (and the 100% of their code) should be testeable without need
 * of Arduino specific HW.
 */

#ifndef MAIN_INC
#define MAIN_INC

#define CLASS_MAIN "MA"

#include "Images.h"
#include "main4ino/Misc.h"
#include <HttpCodes.h>
#include <Module.h>

#ifndef PROJ_VERSION
#error "No PROJ_VERSION defined"
#endif // PROJ_VERSION

#ifndef FRAG_TO_SLEEP_MS_MAX
#define FRAG_TO_SLEEP_MS_MAX 1000 // maximum sleeping time for which the module can be unresponsive
#endif // FRAG_TO_SLEEP_MS_MAX

#ifndef PERIOD_SEC
#define PERIOD_SEC 60
#endif // PERIOD_SEC

#define PERIOD_MSEC (PERIOD_SEC * 1000)

#define COMMAND_MAX_LENGTH 64

#define WAIT_BEFORE_HTTP_MS 1500

#define URL_PRINT_MAX_LENGTH 20

#ifndef USER_DELAY_MS
#define USER_DELAY_MS 3000
#endif // USER_DELAY_MS

#ifndef WIFI_DELAY_MS
#define WIFI_DELAY_MS 3000
#endif // WIFI_DELAY_MS

///////////////////////////////
// Provided by generic Main
///////////////////////////////

void setup();
void loop();
bool initWifiInit();
bool initWifiSteady();
void command(const char *cmd);
void messageFuncExt(int line, int size, const char *format, ...);
///////////////////////////////
// To be provided by the Main of a specific architecture
///////////////////////////////

// Setup step specific to the architecture
void setupArchitecture();

// Loop specific to the architecture
void loopArchitecture();

// Setup wifi using provided parameters
bool initWifi(const char *ssid, const char *pass, bool skipIfAlreadyConnected, int retries);

// Interruptable sleep function (haveToInterrupt called within).
void sleepInterruptable(unsigned long cycleBegin, unsigned long periodMs);

// Function to execute whenever a button is pressed (interrupt handling)
bool haveToInterrupt();

// Message function. Directly connected with user.
void messageFunc(int line, const char *str, int size);

// Log function.
void logLine(const char *str);

// Arms control function.
void arms(int left, int right, int steps);

// HTTP GET function.
int httpGet(const char *url, ParamStream *response);

// HTTP POST function.
int httpPost(const char *url, const char *body, ParamStream *response);

// IO control function.
void ios(char led, bool v);

// Show an image (either a catalog image or a custom bitmap)
void lcdImg(char img, uint8_t bitmap[]);

// Retrieve the estimation of the current milliseconds since the boot
uint32_t cmillis();

#endif // MAIN_INC
