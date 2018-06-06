#ifdef UNIT_TEST

// Auxiliary libraries
#include <string.h>
#include <unity.h>
#include <main4ino/Misc.h>

// Being tested
#include <actors/Body.h>

int faceCleared = 0;
char lastMsg[100];
char lastArms[100];
bool ledY;
bool ledR;
bool ledW;
bool fan;

void setUp() {
  faceCleared = 0;
  lastMsg[0] = 0;
  lastArms[0] = 0;
  ledY = false;
  ledR = false;
  ledW = false;
  fan = false;
}

void tearDown() {}

void lcdImg(char img, uint8_t bitmap[]) {
  switch (img) {
  	case 'b':
      faceCleared++;
      break;
  	default:
  		break;
  }
}

void messageOnLcd(int line, const char *str, int s) {
  strcpy(lastMsg, str);
}

void arms(int left, int right, int steps) {
	sprintf(lastArms, "left:%d,right:%d,steps:%d", left, right, steps);
}

void led(char led, bool v) {
	switch(led) {
		case 'y':
			ledY = v;
			break;
		case 'w':
			ledW = v;
			break;
		case 'r':
			ledR = v;
			break;
		case 'f':
			fan = v;
			break;
		default:
			break;
	}
}

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
  TEST_ASSERT_EQUAL_STRING("", lastMsg);

  Timing *t = b.getFrequencyConfiguration();
  t->setCurrentTime(3600 * 2 + 60 * 33 + 10);
  b.act();

  TEST_ASSERT_EQUAL(t->getCurrentTime(), faceCleared);
  TEST_ASSERT_EQUAL_STRING("02:33", lastMsg);
}

void executeMove(Body* b, const char* move) {
  Buffer<20> mv0;
  mv0.fill(move);
  b->setProp(BodyConfigMove0, SetValue, &mv0, NULL);
  Timing *t = b->getFrequencyConfiguration();
  t->setCurrentTime(t->getCurrentTime() + 1); // assumes configured to act every second
  b->act();
}

void test_body_performs_basic_moves() {

  Body b("b");
  Quotes q("q");
  initBody(&b, &q);

  Long time0(201010101);      // act every single second / act() method call
  b.setProp(BodyConfigTime0, SetValue, &time0, NULL);

  TEST_ASSERT_EQUAL_STRING("", lastArms);

  executeMove(&b, "A91");
  TEST_ASSERT_EQUAL_STRING("left:9,right:1,steps:20", lastArms);

  executeMove(&b, "B56");
  TEST_ASSERT_EQUAL_STRING("left:5,right:6,steps:40", lastArms);

  executeMove(&b, "C13");
  TEST_ASSERT_EQUAL_STRING("left:1,right:3,steps:100", lastArms);

  executeMove(&b, "Lyn");
  TEST_ASSERT_EQUAL(false, ledY);

  executeMove(&b, "Lyy");
  TEST_ASSERT_EQUAL(true, ledY);

  executeMove(&b, "Lfn");
  TEST_ASSERT_EQUAL(false, fan);

  executeMove(&b, "Lfy");
  TEST_ASSERT_EQUAL(true, fan);

  executeMove(&b, "Zz.");
  TEST_ASSERT_EQUAL_STRING("left:0,right:0,steps:100", lastArms);
  TEST_ASSERT_EQUAL(false, ledY);
  TEST_ASSERT_EQUAL(false, ledR);
  TEST_ASSERT_EQUAL(false, ledW);
  TEST_ASSERT_EQUAL(false, fan);

}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_body_shows_time);
  RUN_TEST(test_body_performs_basic_moves);
  return (UNITY_END());
}

#endif // UNIT_TEST
