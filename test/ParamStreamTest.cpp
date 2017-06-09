#ifdef UNIT_TEST

// Auxiliary libraries
#include <unity.h>
#include <string.h>

// Library being tested
#include <actors/ParamStream.h>

void setUp() {}

void tearDown() {}

void feed(ParamStream* ps, const char* msg) {
  for (int i=0; i<strlen(msg); i++) {
    ps->write(msg[i]);
  }
}


void test_param_stream_behaviour() {
  ParamStream ps;

  feed(&ps, "{}");

  TEST_ASSERT_EQUAL(2, ps.available());

  ps.flush();

  feed(&ps, "{\"key\":{\"innerkey\":\"innervalue\"}}");
  JsonObject& json = ps.parse();
  TEST_ASSERT_EQUAL_STRING("innervalue", json["key"]["innerkey"]);

  ps.flush();
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_param_stream_behaviour);
  UNITY_END();
}

#endif // UNIT_TEST
