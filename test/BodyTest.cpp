#ifdef UNIT_TEST

// Auxiliary libraries
#include <unity.h>
#include <string.h>

// Being tested
#include <actors/Body.h>

int faceCleared = 0;
char msg[100];

void setUp() {
  faceCleared = 0;
  msg[0] = 0;
}

void tearDown() {}

void beClear() {
  faceCleared++;
}
void beSmily() {}
void beSad() {}
void beNormal() {}
void beSleepy() {}
void messageOnLcd(int line, const char *str, int s) {
  strcpy(msg, str);
}
void arms(ArmState left, ArmState right) {}
void led(unsigned char led, unsigned char v) {}

void test_body_shows_time() {

  setLogLevel(Warn);

  Body b("b");

  b.setSmilyFace(beSmily);
  b.setSadFace(beSad);
  b.setNormalFace(beNormal);
  b.setSleepyFace(beSleepy);
  b.setClearFace(beClear);
  b.setArms(arms);
  b.setMessageFunc(messageOnLcd);
  b.setIosFunc(led);

  Long time0(201010101);      // every single second
  Buffer<10> move0("mcXfcX"); // clock message (show current time) and face cleared

  b.setProp(BodyConfigTime0, SetValue, &time0, NULL);
  b.setProp(BodyConfigMove0, SetValue, &move0, NULL);

  TEST_ASSERT_EQUAL(0, faceCleared);
  TEST_ASSERT_EQUAL_STRING("", msg);

  Timing *t = b.getFrequencyConfiguration();
  t->setCurrentTime(3600 * 2 + 60 * 33 + 10);
  b.act();

  TEST_ASSERT_EQUAL(t->getCurrentTime(), faceCleared);
  TEST_ASSERT_EQUAL_STRING("02:33", msg);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_body_shows_time);
  return (UNITY_END());
}

#endif // UNIT_TEST
