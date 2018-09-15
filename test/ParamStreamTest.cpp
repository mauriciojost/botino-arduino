#ifdef UNIT_TEST

// Auxiliary libraries
#include <string.h>
#include <unity.h>

// Library being tested
#include <actors/ParamStream.h>

void setUp() {}

void tearDown() {}

void feed(ParamStream *ps, const char *msg) {
  for (int i = 0; i < strlen(msg); i++) {
    ps->write(msg[i]);
  }
}

void test_param_stream_behaviour() {
  Buffer<MAX_JSON_STR_LENGTH> bytesReceived;
  ParamStream ps(&bytesReceived);

  feed(&ps, "{}");

  TEST_ASSERT_EQUAL(2, ps.available());
}

void test_param_stream_behaviour2() {
  Buffer<MAX_JSON_STR_LENGTH> bytesReceived;
  ParamStream ps(&bytesReceived);
  feed(&ps, "{\"key\":{\"innerkey\":\"innervalue\"}}");
  JsonObject &json = ps.parse();
  TEST_ASSERT_EQUAL_STRING("innervalue", json["key"]["innerkey"]);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_param_stream_behaviour);
  RUN_TEST(test_param_stream_behaviour2);
  return (UNITY_END());
}

#endif // UNIT_TEST
