#ifdef UNIT_TEST

// Auxiliary libraries
#include <unity.h>
#include <string.h>

// Being tested
#include <actors/sync/SetupSync.h>
#include <main4ino/SerBot.h>
#include <main4ino/Array.h>

void setUp() {}

void tearDown() {}

bool initWifiInit() {
  return true; // always connected
}

bool initWifiSteady() {
  return true; // always connected
}

int httpGet(const char *url, ParamStream *response) {

  if (strcmp("http://dweet.io/get/latest/dweet/for/device1-clock-target", url) == 0) {
    response->append("{\"with\":[{\"content\":{\"ssid\":\"pepe\", \"pass\":\"koko\"}}]}");
  }
  return 1;
}

void test_setupsync_syncs_properties() {

  setLogLevel(Debug);
  Clock clock("clock");

  Array<Actor *> actors(1);
  actors.set(0, &clock);

  SerBot b(&clock, &actors);

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
