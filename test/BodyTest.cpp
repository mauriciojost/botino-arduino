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

void lcdImg(char img, uint8_t bitmap[]) {
  if (img == 'b') {
    faceCleared++;
  }
}

void messageOnLcd(int line, const char *str, int s) {
  strcpy(msg, str);
}
void arms(int left, int right, int steps) {}
void led(char led, bool v) {}

void initBody(Body *b, Quotes *q) {
  b->setLcdImgFunc(lcdImg);
  b->setArmsFunc(arms);
  b->setMessageFunc(messageOnLcd);
  b->setIosFunc(led);
  b->setQuotes(q);
}

void test_body_shows_time() {

  setLogLevel(Warn);

  Body b("b");
  Quotes q("q");
  initBody(&b, &q);

  Long time0(201010101);      // every single second
  Buffer<10> move0("Mc.Fb."); // clock message (show current time) and face black

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
