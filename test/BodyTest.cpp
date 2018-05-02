#ifdef UNIT_TEST

// Auxiliary libraries
#include <unity.h>
#include <string.h>

// Being tested
#include <actors/Body.h>

void setUp() {}

void tearDown() {}

int faceCleared = 0;

void beClear() {
	faceCleared++;
}
void beSmily() { }
void beSad() { }
void beNormal() { }
void beSleepy() { }
void messageOnLcd(int line, const char *str) { }
void arms(ArmState left, ArmState right) { }
void led(unsigned char led, unsigned char v) { }

void logLine(const char *str) {
	printf("%s\n", str);
}

void test_body_shows_time() {

  //setupLog(logLine);

  Body b("b");

  b.setSmilyFace(beSmily);
  b.setSadFace(beSad);
  b.setNormalFace(beNormal);
  b.setSleepyFace(beSleepy);
  b.setClearFace(beClear);
  b.setArms(arms);
  b.setMessageFunc(messageOnLcd);
  b.setLedFunc(led);

  Long time0(201010101); // every single second
  Buffer<10> move0("fcX"); // face clear

  b.setProp(BodyConfigTime0, SetValue, &time0, NULL);
  b.setProp(BodyConfigMove0, SetValue, &move0, NULL);

  TEST_ASSERT_EQUAL(0, faceCleared);

  Timing* t = b.getFrequencyConfiguration();
  t->setCurrentTime(1);
  b.act();

  TEST_ASSERT_EQUAL(1, faceCleared);

}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_body_shows_time);
  return (UNITY_END());
}

#endif // UNIT_TEST
