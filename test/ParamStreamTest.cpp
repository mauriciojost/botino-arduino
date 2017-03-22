#ifdef UNIT_TEST
#ifdef PARAM_STREAM_TEST

// Auxiliary libraries
#include <unity.h>
#include <string.h>

// Library being tested
#include <actors/ParamStream.h>

void setUp(void) {}

void tearDown(void) {}

void feed(ParamStream* ps, const char* msg) {
  for (int i=0; i<strlen(msg); i++) {
    ps->write(msg[i]);
  }
}


void test_param_stream_behaviour(void) {
  char newValue[100];

  ParamStream ps;

  feed(&ps, "c001p11=55a&");
  TEST_ASSERT_EQUAL(1, ps.getNroCommandsAvailable());
  TEST_ASSERT_EQUAL(1, ps.getCommands()[0].confIndex);
  TEST_ASSERT_EQUAL(11, ps.getCommands()[0].propIndex);
  ps.getCommands()[0].newValue.save(newValue);
  TEST_ASSERT_EQUAL_STRING("55a", newValue);
  ps.flush();

  //feed(&ps, "c0k0111=55a&");
  //TEST_ASSERT_EQUAL(0, ps.getNroCommandsAvailable());

}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_param_stream_behaviour);
  UNITY_END();
}

#endif // PARAM_STREAM_TEST
#endif // UNIT_TEST
