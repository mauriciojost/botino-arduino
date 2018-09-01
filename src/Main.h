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
#endif                            // FRAG_TO_SLEEP_MS_MAX

#ifndef PERIOD_SEC
#define PERIOD_SEC 60
#endif // PERIOD_SEC

#define PERIOD_MSEC (PERIOD_SEC * 1000)

#define PERIOD_CONFIGURE_MSEC 4000

#define WAIT_BEFORE_HTTP_MS 100

#ifndef URL_PRINT_MAX_LENGTH
#define URL_PRINT_MAX_LENGTH 20
#endif // URL_PRINT_MAX_LENGTH

#ifndef USER_DELAY_MS
#define USER_DELAY_MS 3000
#endif // USER_DELAY_MS

#ifndef WIFI_DELAY_MS
#define WIFI_DELAY_MS 3000
#endif // WIFI_DELAY_MS

//////////////////////////////////////////////////////////////
// Provided by generic Main
//////////////////////////////////////////////////////////////

// Standard arduino setup
void setup();

// Standard arduino loop
void loop();

// Initialize init  wifi
bool initWifiInit();

// Initialize steady wifi
bool initWifiSteady();

// Extended "message to user" function (printf way)
void messageFuncExt(int line, int size, const char *format, ...);

//////////////////////////////////////////////////////////////
// To be provided by the Main of a specific architecture
//////////////////////////////////////////////////////////////

// Callbacks
///////////////////

// The log function (that will print to screen, Serial, telnet, or whatever wished)
void logLine(const char *str);

// Setup wifi using provided parameters
bool initWifi(const char *ssid, const char *pass, bool skipIfAlreadyConnected, int retries);

// HTTP GET function.
int httpGet(const char *url, ParamStream *response);

// HTTP POST function.
int httpPost(const char *url, const char *body, ParamStream *response);

// Message function. Directly connected with user.
void messageFunc(int line, const char *str, int size);

// Arms control function.
void arms(int left, int right, int steps);

// IO control function.
void ios(char led, bool v);

// Clear device (for development purposes, to clear logs, stacktraces, etc)
void clearDevice();

// Show an image (either a catalog image or a custom bitmap)
void lcdImg(char img, uint8_t bitmap[]);

// Execution
///////////////////

// Interruptable sleep function (haveToInterrupt called within).
void sleepInterruptable(unsigned long cycleBegin, unsigned long periodMs);

// Function to execute whenever a button is pressed (interrupt handling)
bool haveToInterrupt();

// Setup step specific to the architecture
void setupArchitecture();

// Loop in run mode specific to the architecture
void runModeArchitecture();

// Loop in configure mode specific to the architecture
void configureModeArchitecture();

#endif // MAIN_INC
