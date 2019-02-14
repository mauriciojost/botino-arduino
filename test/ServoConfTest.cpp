#ifdef UNIT_TEST

// Auxiliary libraries
#include <main4ino/Misc.h>
#include <string.h>
#include <unity.h>

// Being tested
#include <ServoConf.h>

void setUp() {}

void tearDown() {}

void test_basic_behaviour() {
  // one scenario
  ServoConf s("0/0/0"); // base, range, inversion

  // raw basics
  TEST_ASSERT_EQUAL(0, s.getBaseDegrees());
  TEST_ASSERT_EQUAL(0, s.getRangeDegrees());
  TEST_ASSERT_EQUAL(false, s.getInvert());

  // derived basics
  TEST_ASSERT_EQUAL(0.0f, s.getStepDegrees());
  TEST_ASSERT_EQUAL(0, s.getTargetDegreesFromPosition(0));
  TEST_ASSERT_EQUAL(0, s.getTargetDegreesFromPosition(9));

  // another scenario
  ServoConf t("10/0/1"); // base, range, inversion

  // raw basics
  TEST_ASSERT_EQUAL(10, t.getBaseDegrees());
  TEST_ASSERT_EQUAL(0, t.getRangeDegrees());
  TEST_ASSERT_EQUAL(true, t.getInvert());

  // derived basics
  TEST_ASSERT_EQUAL(0.0f, t.getStepDegrees());
  TEST_ASSERT_EQUAL(10, t.getTargetDegreesFromPosition(0));
  TEST_ASSERT_EQUAL(10, t.getTargetDegreesFromPosition(9));
}

void test_non_basic_behaviour() {
  // one scenario
  ServoConf s("0/9/0"); // base, range, inversion

  // raw basics
  TEST_ASSERT_EQUAL(0, s.getBaseDegrees());
  TEST_ASSERT_EQUAL(9, s.getRangeDegrees());
  TEST_ASSERT_EQUAL(false, s.getInvert());

  // derived basics
  TEST_ASSERT_EQUAL(1.0f, s.getStepDegrees());
  TEST_ASSERT_EQUAL(0, s.getTargetDegreesFromPosition(0));
  TEST_ASSERT_EQUAL(9, s.getTargetDegreesFromPosition(9));

  // another scenario
  ServoConf t("0/9/1"); // base, range, inversion

  // raw basics
  TEST_ASSERT_EQUAL(0, t.getBaseDegrees());
  TEST_ASSERT_EQUAL(9, t.getRangeDegrees());
  TEST_ASSERT_EQUAL(true, t.getInvert());

  // derived basics
  TEST_ASSERT_EQUAL(1.0f, t.getStepDegrees());
  TEST_ASSERT_EQUAL(9, t.getTargetDegreesFromPosition(0));
  TEST_ASSERT_EQUAL(0, t.getTargetDegreesFromPosition(9));

  // another scenario
  ServoConf u("100/90/0"); // base, range, inversion

  // raw basics
  TEST_ASSERT_EQUAL(100, u.getBaseDegrees());
  TEST_ASSERT_EQUAL(90, u.getRangeDegrees());
  TEST_ASSERT_EQUAL(false, u.getInvert());

  // derived basics
  TEST_ASSERT_EQUAL(10.0f, u.getStepDegrees());
  TEST_ASSERT_EQUAL(100, u.getTargetDegreesFromPosition(0));
  TEST_ASSERT_EQUAL(190, u.getTargetDegreesFromPosition(9));

  // another scenario
  ServoConf v("100/90/1"); // base, range, inversion

  // raw basics
  TEST_ASSERT_EQUAL(100, v.getBaseDegrees());
  TEST_ASSERT_EQUAL(90, v.getRangeDegrees());
  TEST_ASSERT_EQUAL(true, v.getInvert());

  // derived basics
  TEST_ASSERT_EQUAL(10.0f, v.getStepDegrees());
  TEST_ASSERT_EQUAL(190, v.getTargetDegreesFromPosition(0));
  TEST_ASSERT_EQUAL(100, v.getTargetDegreesFromPosition(9));

  // another scenario
  ServoConf w("0/200/1"); // base, range, inversion

  // raw basics
  TEST_ASSERT_EQUAL(0, w.getBaseDegrees());
  TEST_ASSERT_EQUAL(200, w.getRangeDegrees());
  TEST_ASSERT_EQUAL(true, w.getInvert());

  // derived basics
  TEST_ASSERT_EQUAL(200, w.getTargetDegreesFromPosition(0));
  TEST_ASSERT_EQUAL(0, w.getTargetDegreesFromPosition(9));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_basic_behaviour);
  RUN_TEST(test_non_basic_behaviour);
  return (UNITY_END());
}

#endif // UNIT_TEST
