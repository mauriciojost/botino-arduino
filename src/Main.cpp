/**
 * This file contains:
 * - entry point for arduino programs (setup and loop functions)
 * - declaration of HW specific functions (the definition is in a dedicated file)
 * - other functions that are not defined by HW specific but that use them, that are required by the module
 *   (so that it can be passed as callback).
 * The rest should be put in Module so that they can be tested regardless of the HW used behind.
 */

#define CLASS_MAIN "MA"

#include "Io.h"
#include <Module.h>
#include <main4ino/HttpCodes.h>
#include <main4ino/Misc.h>
#include <main4ino/Table.h>

#ifndef PROJ_VERSION
#error "No PROJ_VERSION defined"
#endif // PROJ_VERSION

#ifndef FRAG_TO_SLEEP_MS_MAX
#define FRAG_TO_SLEEP_MS_MAX 1000 // maximum sleeping time for which the module can be unresponsive
#endif                            // FRAG_TO_SLEEP_MS_MAX

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

#define HEADERS_MAX 3
#define HEADERS_KEY_LENGTH 16
#define HEADERS_VAL_LENGTH 32

//////////////////////////////////////////////////////////////
// Provided by generic Main
//////////////////////////////////////////////////////////////

// Standard arduino setup
void setup();

// Standard arduino loop
void loop();

//////////////////////////////////////////////////////////////
// To be provided by the Main of a specific architecture
//////////////////////////////////////////////////////////////

// Callbacks
///////////////////

// The log function (that will print to screen, Serial, telnet, or whatever wished)
// It should not include "\n" ending as the log4ino library handles newline addition.
void logLine(const char *str);

// Setup wifi using provided parameters
bool initWifi(const char *ssid, const char *pass, bool skipIfAlreadyConnected, int retries);

// HTTP GET function.
int httpGet(const char *url, ParamStream *response, Table *headers);

// HTTP POST function.
int httpPost(const char *url, const char *body, ParamStream *response, Table *headers);

// Message function. Directly connected with user.
void messageFunc(int x, int y, int color, bool wrap, bool clear, int size, const char *str);

// Arms control function.
void arms(int left, int right, int steps);

// IO control function.
void ios(char led, int v);

// Clear device (for development purposes, to clear logs, stacktraces, etc)
void clearDevice();

// Show an image (either a catalog image or a custom bitmap)
void lcdImg(char img, uint8_t bitmap[]);

// Read a file from the filesystem (returns true if success)
bool readFile(const char* fname, Buffer* content);

// Write a file to the filesystem (returns true if success)
bool writeFile(const char* fname, const char* content);

// Execution
///////////////////

// Interruptable sleep function (haveToInterrupt called within).
void sleepInterruptable(time_t cycleBegin, time_t periodSec);

// Function to execute whenever a button is pressed (interrupt handling)
bool haveToInterrupt();

// Setup step specific to the architecture
void setupArchitecture();

// Loop in run mode specific to the architecture
void runModeArchitecture();

// Loop in configure mode specific to the architecture
void configureModeArchitecture();

// Abort execution (non-recoverable-error)
void abort(const char* msg);


Module m;

#ifdef ARDUINO // on ESP8266
#include <MainESP8266.h>
#else   // on PC
#include <MainX86_64.h>
#endif // ARDUINO

bool initWifiSimple() {
  Settings *s = m.getSettings();
  log(CLASS_MAIN, Info, "W.steady");
  bool connected = initWifi(s->getSsid(), s->getPass(), true, 10);
  return connected;
}

void setup() {
  m.setup(
  		setupArchitecture,
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
			runModeArchitecture
			);
}

void loop() {
	m.loop();
}

