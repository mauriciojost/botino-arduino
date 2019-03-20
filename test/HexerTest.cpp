#ifdef UNIT_TEST

// Auxiliary libraries
#include <unity.h>

// Being tested
#include <utils/Hexer.h>

void setUp() {
}

void tearDown() {}


void test_hexing() {
  uint8_t b[10];

  Hexer::hexToByte(b, "0000");
  TEST_ASSERT_EQUAL(0, b[0]);
  TEST_ASSERT_EQUAL(0, b[1]);

  Hexer::hexToByte(b, "00010203040506070809");
  TEST_ASSERT_EQUAL(0, b[0]);
  TEST_ASSERT_EQUAL(1, b[1]);
  TEST_ASSERT_EQUAL(2, b[2]);
  TEST_ASSERT_EQUAL(3, b[3]);
  TEST_ASSERT_EQUAL(4, b[4]);
  TEST_ASSERT_EQUAL(5, b[5]);
  TEST_ASSERT_EQUAL(6, b[6]);
  TEST_ASSERT_EQUAL(7, b[7]);
  TEST_ASSERT_EQUAL(8, b[8]);
  TEST_ASSERT_EQUAL(9, b[9]);

  Hexer::hexToByte(b, "100a");
  TEST_ASSERT_EQUAL(16, b[0]);
  TEST_ASSERT_EQUAL(10, b[1]);

}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_hexing);
  return (UNITY_END());
}

#endif // UNIT_TEST
