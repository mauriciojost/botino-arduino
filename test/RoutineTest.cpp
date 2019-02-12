#ifdef UNIT_TEST

// Being tested
#include <actors/Routine.h>

// Extra libraries needed
#include <main4ino/Misc.h>
#include <string.h>
#include <unity.h>


void setUp() { }

void tearDown() {}

void test_basic_behaviour() {
  Routine r0("routine0");
  r0.set("never", "Z.");

  TEST_ASSERT_EQUAL_STRING("Z.", r0.getMove());
  TEST_ASSERT_EQUAL_STRING("never", r0.getTiming()->getFreq());

  Buffer b("@11h00:D0.");
  Routine r1("routine1");
  r1.set(&b);

  TEST_ASSERT_EQUAL_STRING("D0.", r1.getMove());
  TEST_ASSERT_EQUAL_STRING("@11h00", r1.getTiming()->getFreq());

}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_basic_behaviour);
  return (UNITY_END());
}

#endif // UNIT_TEST
