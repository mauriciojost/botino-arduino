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

#define MV2 "Da2"

const char *replyBody =             "{"
                                    "\"r0\":\"201016060:A99A55W5.W5.A99A55A00F2.W4.M02W5.Mc4W2.\","
                                    "\"r1\":\"201016060:A90A09W5.W5.A09A55A00F1.W4.M12W5.Mc4W2.\","
                                    "\"r2\":\"201016060:" MV2 "\","
                                    "\"r3\":\"201016060:"
                                    "W5.W5.W5.W5.W5.W5.Fw.Fb.M32W5.Mc4W2."
                                    "\""
                                    "}";

void setUp(void) {}

void tearDown() {}

bool initWifi() {
  return true; // always connected
}

int httpGet(const char *url, ParamStream *response) {
	printf("%s\n", url);
  if (strcmp("http://localhost:8080/api/v1/devices/device1/actors/body/reports/last", url) == 0) {
    response->fill("{}");
    return HTTP_OK;
  } else if (strcmp("http://localhost:8080/api/v1/devices/device1/targets/count?status=C", url) == 0) {
    response->fill("{\"count\":1}");
    return HTTP_OK;
  } else if (strcmp("http://localhost:8080/api/v1/devices/device1/actors/body/targets/summary?consume=true&status=C", url) == 0) {
    response->fill(replyBody);
    return HTTP_OK;
  } else {
    return HTTP_BAD_REQUEST;
  }
}

int httpPost(const char *url, const char *body, ParamStream *response) {
  return HTTP_CREATED;
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

  p.getTiming()->setFrek(201010101); // every second

  p.act(); // restore
  p.act(); // load target

  TEST_ASSERT_EQUAL_STRING(MV2, body.getMove(2));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_propsync_syncs_several_body_properties);
  return (UNITY_END());
}

#endif // UNIT_TEST
