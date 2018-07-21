#ifdef UNIT_TEST

// Auxiliary libraries
#include <actors/Body.h>
#include <main4ino/Array.h>
#include <main4ino/Misc.h>
#include <main4ino/SerBot.h>
#include <string.h>
#include <unity.h>

// Being tested
#include <actors/PropSync.h>

#define JSON_PREFIX "{\"with\":[{\"content\":"
#define JSON_SUFFIX "}]}"

#define MV2 "A90A09W5."

const char *replyBody = JSON_PREFIX "{"
                                    "\"mv0\":\"A99A55W5.W5.A99A55A00F2.W4.M02W5.Mc4W2.\","
                                    "\"mv1\":\"A90A09W5.W5.A09A55A00F1.W4.M12W5.Mc4W2.\","
                                    "\"mv2\":\"" MV2 "\","
                                    "\"mv3\":\""
                                    "W5.W5.W5.W5.W5.W5.Fw.Fb.M32W5.Mc4W2."
                                    "\","
                                    "\"t0\":\"201016060\","
                                    "\"t1\":\"201016060\","
                                    "\"t2\":\"201016060\","
                                    "\"t3\":\"201016060\""
                                    "}" JSON_SUFFIX;

void setUp(void) {}

void tearDown() {}

bool initWifi() {
  return true; // always connected
}

int httpGet(const char *url, ParamStream *response) {
  if (strcmp("http://dweet.io/get/latest/dweet/for/device1-body-target", url) == 0) {
    response->fill(replyBody);
    return HTTP_OK;
  }
  return HTTP_BAD_REQUEST;
}

int httpPost(const char *url, const char *body, ParamStream *response) {
  return HTTP_OK;
}

void test_propsync_syncs_several_body_properties() {
  Clock clock("clock");
  Body body("body");

  Array<Actor *> actors(2);
  actors.set(0, &clock);
  actors.set(1, &body);

  SerBot b(&clock, &actors);

  PropSync p("p");
  p.setBot(&b);

  p.setInitWifi(initWifi);
  p.setHttpGet(httpGet);
  p.setHttpPost(httpPost);

  p.getFrequencyConfiguration()->setFrequency(OnceEvery1Second);

  p.act();

  TEST_ASSERT_EQUAL_STRING(MV2, body.getMove(2));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_propsync_syncs_several_body_properties);
  return (UNITY_END());
}

#endif // UNIT_TEST
