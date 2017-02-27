#ifdef UNIT_TEST
#ifdef PARAM_STREAM_TEST

// Auxiliary libraries
#include <unity.h>
#include <string.h>

// Library being tested
#include <actors/ParamStream.h>

void setUp(void) {}

void tearDown(void) {}

void test_param_stream_behaviour(void) {
  const char* msg = "c0p1=5";
  ParamStream ps;
  for (int i=0; i<strlen(msg); i++) {
    ps.write(msg[i]);
  }
  TEST_ASSERT_EQUAL(1, ps.getNroCommandsAvailable());
  TEST_ASSERT_EQUAL(0, ps.getCommands()[0].configurableIndex);
  TEST_ASSERT_EQUAL(1, ps.getCommands()[0].propertyIndex);
  TEST_ASSERT_EQUAL(5, ps.getCommands()[0].newValue);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_param_stream_behaviour);
  UNITY_END();
}

#endif // PARAM_STREAM_TEST
#endif // UNIT_TEST
