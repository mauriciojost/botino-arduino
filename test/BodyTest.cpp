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
int ledY;
int ledR;
int ledW;
int fan;

void setUp() {
  faceCleared = 0;
  lastMsg[0] = 0;
  lastArms[0] = 0;
  ledY = 0;
  ledR = 0;
  ledW = 0;
  fan = 0;
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

void messageOnLcd(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str) {
	if (y == 0) { // ignore messages that do not belong to body
    strcpy(lastMsg, str);
	}
}

void arms(int left, int right, int steps) {
  sprintf(lastArms, "left:%d,right:%d,steps:%d", left, right, steps);
}

void led(char led, IoMode v) {
  switch (led) {
    case 'y':
      ledY = (int)v;
      break;
    case 'w':
      ledW = (int)v;
      break;
    case 'r':
      ledR = (int)v;
      break;
    case 'f':
      fan = (int)v;
      break;
    default:
      break;
  }
}

void initBody(Body *b, Quotes *q, Images *i, Ifttt *it, Notifier *n) {
  b->setArmsFunc(arms);
  b->setNotifier(n);
  b->setIosFunc(led);
  b->setQuotes(q);
  b->setImages(i);
  b->setIfttt(it);
}

void test_body_shows_time() {

  Quotes q("q");
  Notifier n("n");
  n.setMessageFunc(messageOnLcd);
  n.setLcdImgFunc(lcdImg);
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it, &n);

  Buffer move0(20, "~1s:Mc1.Fb."); // clock message (show current time) and face black

  b.setPropValue(BodyRoutine0Prop, &move0);

  TEST_ASSERT_EQUAL(0, faceCleared);
  TEST_ASSERT_EQUAL_STRING("", lastMsg);

  Timing *t = b.getTiming();
  t->setCurrentTime(3600 * 2 + 60 * 33 + 10);
  b.act();

  TEST_ASSERT_EQUAL_STRING("02:33", lastMsg);
  TEST_ASSERT_EQUAL(1, faceCleared); // 1 act

  b.act();

  TEST_ASSERT_EQUAL(2, faceCleared); // 2 acts
}

void executeMove(Body *b, const char *move) {
  Buffer mv0(20);
  mv0.fill(move);
  b->setPropValue(BodyRoutine0Prop, &mv0);
  Timing *t = b->getTiming();
  t->setCurrentTime(t->getCurrentTime() + 1); // assumes configured to act every second
  b->act();
}

void setMove(Body *b, int i, const char *move) {
  Buffer mv(30);
  mv.fill(move);
  b->setPropValue(BodyRoutine0Prop + i, &mv);
}

void test_body_performs_basic_moves() {

  Quotes q("q");
  Notifier n("n");
  n.setMessageFunc(messageOnLcd);
  n.setLcdImgFunc(lcdImg);
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it, &n);

  TEST_ASSERT_EQUAL_STRING("", lastArms);

  executeMove(&b, "~1s:A91.");
  TEST_ASSERT_EQUAL_STRING("left:9,right:1,steps:20", lastArms);

  executeMove(&b, "~1s:B56.");
  TEST_ASSERT_EQUAL_STRING("left:5,right:6,steps:40", lastArms);

  executeMove(&b, "~1s:C13.");
  TEST_ASSERT_EQUAL_STRING("left:1,right:3,steps:100", lastArms);

  executeMove(&b, "~1s:Lyn.");
  TEST_ASSERT_EQUAL(0, ledY);

  executeMove(&b, "~1s:Lyy.");
  TEST_ASSERT_EQUAL(1, ledY);

  executeMove(&b, "~1s:Lfn.");
  TEST_ASSERT_EQUAL(0, fan);

  executeMove(&b, "~1s:Lfy.");
  TEST_ASSERT_EQUAL(1, fan);

  executeMove(&b, "~1s:Z.");
  TEST_ASSERT_EQUAL_STRING("left:0,right:0,steps:40", lastArms);
  TEST_ASSERT_EQUAL(0, ledY);
  TEST_ASSERT_EQUAL(0, ledR);
  TEST_ASSERT_EQUAL(0, ledW);
  TEST_ASSERT_EQUAL(0, fan);

  executeMove(&b, "~1s:M1HEY.");
  TEST_ASSERT_EQUAL_STRING("HEY", lastMsg);
}

void test_body_creates_predictions() {

  Quotes q("q");
  Notifier n("n");
  n.setMessageFunc(messageOnLcd);
  n.setLcdImgFunc(lcdImg);
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it, &n);

  executeMove(&b, "~1s:Mp1.");
  TEST_ASSERT_EQUAL_STRING("your colleague will ride your colleague at work", lastMsg);
}

void test_body_parses_moves() {

  Quotes q("q");
  Notifier n("n");
  n.setMessageFunc(messageOnLcd);
  n.setLcdImgFunc(lcdImg);
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it, &n);

  TEST_ASSERT_EQUAL_STRING("", lastArms);

  // Move correctly formed
  TEST_ASSERT_EQUAL_STRING("Z.", b.performPose("Mp1.Z.")); // consume 1 pose at a time
  TEST_ASSERT_EQUAL_STRING("", b.performPose("Z."));
  TEST_ASSERT_EQUAL(NULL, b.performPose(""));

  // Move malformed (does not end in .)
  TEST_ASSERT_EQUAL_STRING("Z", b.performPose("Mp1.Z")); // consume 1 pose at a time
  TEST_ASSERT_EQUAL(NULL, b.performPose("Z"));
}

void test_body_parses_move_timing_alias() {

  Quotes q("q");
  Notifier n("n");
  n.setMessageFunc(messageOnLcd);
  n.setLcdImgFunc(lcdImg);
  Images im("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &im, &it, &n);

  for (int i = 0; i < NRO_ROUTINES; i++) {
    setMove(&b, i, "~1s:B56.");
    TEST_ASSERT_EQUAL_STRING("B56.", b.getMove(i));

    setMove(&b, i, "hourly:B22.");
    TEST_ASSERT_EQUAL_STRING("B22.", b.getMove(i));
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_body_shows_time);
  RUN_TEST(test_body_performs_basic_moves);
  RUN_TEST(test_body_creates_predictions);
  RUN_TEST(test_body_parses_moves);
  RUN_TEST(test_body_parses_move_timing_alias);
  return (UNITY_END());
}

#endif // UNIT_TEST
