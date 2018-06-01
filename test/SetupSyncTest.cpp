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

int httpGetMockPassHello(const char *url, ParamStream *response) {
  if (strcmp("http://dweet.io/get/latest/dweet/for/device1-setup", url) == 0) {
    // Pass generated using function AES mode ECB with incremental hex key 000102...0f from http://aes.online-domain-tools.com/
    response->fill(
        "{\"with\":[{\"content\":{\"ssid\":\"7d6a85f1c257d7e64e54f095b14f2338\", \"pass\":\"a3a5fcf64804dbb99b2781aebfe338c9\"}}]}");
  } else {
    log(LOG_CLASS, Error, "Unknown: %s", url);
  }
  return 1;
}

int httpGetMockPassHelloMyLittleDarling(const char *url, ParamStream *response) {
  if (strcmp("http://dweet.io/get/latest/dweet/for/device1-setup", url) == 0) {
    // Pass generated using function AES mode ECB with incremental hex key 000102...0f from http://aes.online-domain-tools.com/
    response->fill("{\"with\":[{\"content\":{\"ssid\":\"7d6a85f1c257d7e64e54f095b14f2338\", "
                   "\"pass\":\"85b22d5b7548a237ba28c87275324e54fab0417e6ea45b3236991933aa04d8af\"}}]}");
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
  p.setHttpGet(httpGetMockPassHello);
  p.setHttpGet(httpGetMockPassHello);

  p.getFrequencyConfiguration()->setFrequency(OnceEvery1Second);

  p.act();

  TEST_ASSERT_EQUAL_STRING("name", p.getSsid());
  TEST_ASSERT_EQUAL_STRING("hello", p.getPass());
}

void test_setupsync_syncs_properties_longer() {

  setLogLevel(Debug);
  SetupSync p("s");

  p.setInitWifiInit(initWifiInit);
  p.setInitWifiSteady(initWifiSteady);
  p.setHttpGet(httpGetMockPassHelloMyLittleDarling);

  p.getFrequencyConfiguration()->setFrequency(OnceEvery1Second);

  p.act();

  TEST_ASSERT_EQUAL_STRING("name", p.getSsid());
  TEST_ASSERT_EQUAL_STRING("hello my little darling", p.getPass());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_setupsync_syncs_properties);
  RUN_TEST(test_setupsync_syncs_properties_longer);
  return (UNITY_END());
}

#endif // UNIT_TEST
