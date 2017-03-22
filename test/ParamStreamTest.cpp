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

  feed(&ps, "c001p11=11a&");
  TEST_ASSERT_EQUAL(1, ps.getNroCommandsAvailable());
  TEST_ASSERT_EQUAL(1, ps.getCommands()[0].confIndex);
  TEST_ASSERT_EQUAL(11, ps.getCommands()[0].propIndex);
  TEST_ASSERT_EQUAL_STRING("11a", ps.getCommands()[0].newValue.getBuffer());
  ps.flush();

  feed(&ps, "c0k0111=55a&");
  TEST_ASSERT_EQUAL(0, ps.getNroCommandsAvailable());
  ps.flush();

  feed(&ps, "&&c0k1=aa&c0p2=bb&c0p3=55a&"); // 3 commands, 1 malformed (c0k1=aa)
  TEST_ASSERT_EQUAL(2, ps.getNroCommandsAvailable());
  TEST_ASSERT_EQUAL(0, ps.getCommands()[0].confIndex);
  TEST_ASSERT_EQUAL(2, ps.getCommands()[0].propIndex);
  TEST_ASSERT_EQUAL_STRING("bb", ps.getCommands()[0].newValue.getBuffer());
  TEST_ASSERT_EQUAL(0, ps.getCommands()[1].confIndex);
  TEST_ASSERT_EQUAL(3, ps.getCommands()[1].propIndex);
  TEST_ASSERT_EQUAL_STRING("55a", ps.getCommands()[1].newValue.getBuffer());
  ps.flush();

}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_param_stream_behaviour);
  UNITY_END();
}

#endif // PARAM_STREAM_TEST
#endif // UNIT_TEST
