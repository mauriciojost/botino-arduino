#ifdef UNIT_TEST

// Auxiliary libraries
#include <main4ino/Misc.h>
#include <string.h>
#include <unity.h>

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
  switch (led) {
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

void initBody(Body *b, Quotes *q, Images *i, Ifttt* it) {
  b->setLcdImgFunc(lcdImg);
  b->setArmsFunc(arms);
  b->setMessageFunc(messageOnLcd);
  b->setIosFunc(led);
  b->setQuotes(q);
  b->setImages(i);
  b->setIfttt(it);
}

void test_body_shows_time() {

  setLogLevel(Warn);
  Quotes q("q");
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it);

  Buffer<20> move0("201010101:Mc1.Fb."); // clock message (show current time) and face black

  b.setPropValue(BodyRoutine0Prop, &move0);

  TEST_ASSERT_EQUAL(0, faceCleared);
  TEST_ASSERT_EQUAL_STRING("", lastMsg);

  Timing *t = b.getTiming();
  t->setCurrentTime(3600 * 2 + 60 * 33 + 10);
  b.act();

  TEST_ASSERT_EQUAL_STRING("02:33", lastMsg);
  TEST_ASSERT_EQUAL(t->getCurrentTime(), faceCleared);
}

void executeMove(Body *b, const char *move) {
  Buffer<20> mv0;
  mv0.fill(move);
  b->setPropValue(BodyRoutine0Prop, &mv0);
  Timing *t = b->getTiming();
  t->setCurrentTime(t->getCurrentTime() + 1); // assumes configured to act every second
  b->act();
}

void test_body_performs_basic_moves() {

  Quotes q("q");
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it);

  TEST_ASSERT_EQUAL_STRING("", lastArms);

  executeMove(&b, "201010101:A91.");
  TEST_ASSERT_EQUAL_STRING("left:9,right:1,steps:20", lastArms);

  executeMove(&b, "201010101:B56.");
  TEST_ASSERT_EQUAL_STRING("left:5,right:6,steps:40", lastArms);

  executeMove(&b, "201010101:C13.");
  TEST_ASSERT_EQUAL_STRING("left:1,right:3,steps:100", lastArms);

  executeMove(&b, "201010101:Lyn.");
  TEST_ASSERT_EQUAL(false, ledY);

  executeMove(&b, "201010101:Lyy.");
  TEST_ASSERT_EQUAL(true, ledY);

  executeMove(&b, "201010101:Lfn.");
  TEST_ASSERT_EQUAL(false, fan);

  executeMove(&b, "201010101:Lfy.");
  TEST_ASSERT_EQUAL(true, fan);

  executeMove(&b, "201010101:Z.");
  TEST_ASSERT_EQUAL_STRING("left:0,right:0,steps:20", lastArms);
  TEST_ASSERT_EQUAL(false, ledY);
  TEST_ASSERT_EQUAL(false, ledR);
  TEST_ASSERT_EQUAL(false, ledW);
  TEST_ASSERT_EQUAL(false, fan);

  executeMove(&b, "201010101:M1HEY.");
  TEST_ASSERT_EQUAL_STRING("HEY", lastMsg);
}

void test_body_creates_predictions() {

  Quotes q("q");
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it);

  executeMove(&b, "201010101:Mp1.");
  TEST_ASSERT_EQUAL_STRING("your colleague will ride your colleague at work", lastMsg);
}

void test_body_parses_moves() {

  Quotes q("q");
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it);

  TEST_ASSERT_EQUAL_STRING("", lastArms);

  // Move correctly formed
  TEST_ASSERT_EQUAL_STRING("Z.", b.performPose("Mp1.Z.")); // consume 1 pose at a time
  TEST_ASSERT_EQUAL_STRING("", b.performPose("Z."));
  TEST_ASSERT_EQUAL(NULL, b.performPose(""));

  // Move malformed (does not end in .)
  TEST_ASSERT_EQUAL_STRING("Z", b.performPose("Mp1.Z")); // consume 1 pose at a time
  TEST_ASSERT_EQUAL(NULL, b.performPose("Z"));

}


int main() {
  UNITY_BEGIN();
  RUN_TEST(test_body_shows_time);
  RUN_TEST(test_body_performs_basic_moves);
  RUN_TEST(test_body_creates_predictions);
  RUN_TEST(test_body_parses_moves);
  return (UNITY_END());
}

#endif // UNIT_TEST
