#ifdef UNIT_TEST
#ifdef PARAM_STREAM_TEST

// Auxiliary libraries
#include <unity.h>

// Library being tested
#include <actors/ParamStream.h>

void setUp(void) {}

void tearDown(void) {}

void test_param_stream_behaviour(void) {
/*
  char buffer[LCD_LENGTH];
  Pump p("PUMP");
  p.setOnValue(PUMP_ON);
  p.setOnValueSilentCycles(2);

  p.setConfig(PumpConfigStateAmount, buffer, SetNext); // DEFAULT_WATER_PUMP_AMOUNT_PER_SHOT + 1

  TEST_ASSERT_EQUAL(PUMP_OFF, p.getActuatorValue());

*/
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_param_stream_behaviour);
  UNITY_END();
}

#endif // PARAM_STREAM_TEST
#endif // UNIT_TEST
