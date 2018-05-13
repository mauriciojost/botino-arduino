#ifdef UNIT_TEST

// Auxiliary libraries
#include <unity.h>
#include <string.h>

// Being tested
#include <actors/sync/SetupSync.h>
#include <main4ino/SerBot.h>
#include <main4ino/Array.h>

#define LOG_CLASS "TST"

void setUp() {}

void tearDown() {}

bool initWifiInit() {
  return true; // always connected
}

bool initWifiSteady() {
  return false; // not connected
}

int httpGet(const char *url, ParamStream *response) {

  if (strcmp("http://dweet.io/get/latest/dweet/for/device1-setup", url) == 0) {
    response->append("{\"with\":[{\"content\":{\"ssid\":\"pepe\", \"pass\":\"c34d4a7d13291613c675d8cc3362a46b\"}}]}");
  } else {
  	log(LOG_CLASS, Error, "Unknown: %s", url);
  }
  return 1;
}

void test_setupsync_syncs_properties() {

  setLogLevel(Debug);

  SetupSync p("s");

  p.setInitWifiInit(initWifiInit);
  p.setInitWifiSteady(initWifiSteady);
  p.setHttpGet(httpGet);

  p.getFrequencyConfiguration()->setFrequency(OnceEvery1Second);

  p.act();

}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_setupsync_syncs_properties);
  return (UNITY_END());
}

#endif // UNIT_TEST
