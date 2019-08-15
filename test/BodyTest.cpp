#ifdef UNIT_TEST

// Auxiliary libraries
#include <main4ino/Misc.h>
#include <string.h>
#include <unity.h>

// Being tested
#include <actors/Body.h>

int faceCleared = 0;
int faceCustom = 0;
char lastMsg[100];
char lastArms[100];
int ledY;
int ledR;
int ledW;
int fan;

void setUp() {
  faceCleared = 0;
  faceCustom = 0;
  lastMsg[0] = 0;
  lastArms[0] = 0;
  ledY = 0;
  ledR = 0;
  ledW = 0;
  fan = 0;
}

void tearDown() {}

bool sleepInt(time_t base, time_t secs) {
  return false;
}

void lcdImg(char img, uint8_t bitmap[]) {
  switch (img) {
    case 'b':
      faceCleared++;
      break;
    case 'c': // custom
      faceCustom++;
      break;
    default:
      break;
  }
}

void messageOnLcd(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str) {
  if (y == 0) { // ignore messages that do not belong to body
    printf("MESSAGE LCD: %s", str);
    strcpy(lastMsg, str);
  }
}

void arms(int left, int right, int factor) {
  sprintf(lastArms, "left:%d,right:%d,factor:%d", left, right, factor);
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
    case '*':
      ledY = (int)v;
      ledW = (int)v;
      ledR = (int)v;
      fan = (int)v;
      break;
    default:
      break;
  }
}

void initBody(Body *b, Quotes *q, Images *i, Ifttt *it, Notifier *n) {
  b->setup(arms, led, sleepInt);
  b->setNotifier(n);
  b->setQuotes(q);
  b->setImages(i);
  b->setIfttt(it);
}

void test_body_shows_time() {

  Quotes q("q");
  Notifier n("n");
  n.setup(lcdImg, messageOnLcd);
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
  n.setup(lcdImg, messageOnLcd);
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it, &n);

  TEST_ASSERT_EQUAL_STRING("", lastArms);

  executeMove(&b, "~1s:A91.");
  TEST_ASSERT_EQUAL_STRING("left:9,right:1,factor:1", lastArms);

  executeMove(&b, "~1s:B56.");
  TEST_ASSERT_EQUAL_STRING("left:5,right:6,factor:2", lastArms);

  executeMove(&b, "~1s:C13.");
  TEST_ASSERT_EQUAL_STRING("left:1,right:3,factor:4", lastArms);

  executeMove(&b, "~1s:Fs.");
  TEST_ASSERT_EQUAL(1, faceCustom);

  executeMove(&b, "~1s:Lyn.");
  TEST_ASSERT_EQUAL(0, ledY);

  executeMove(&b, "~1s:Lyy.");
  TEST_ASSERT_EQUAL(1, ledY);

  executeMove(&b, "~1s:Lfn.");
  TEST_ASSERT_EQUAL(0, fan);

  executeMove(&b, "~1s:Lfy.");
  TEST_ASSERT_EQUAL(1, fan);

  executeMove(&b, "~1s:Z.");
  TEST_ASSERT_EQUAL_STRING("left:0,right:0,factor:2", lastArms);
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
  n.setup(lcdImg, messageOnLcd);
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
  n.setup(lcdImg, messageOnLcd);
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it, &n);

  TEST_ASSERT_EQUAL_STRING("", lastArms);

  PoseExecStatus status = Unknown;
  // Move correctly formed
  TEST_ASSERT_EQUAL_STRING("Z.", b.performPose("Mp1.Z.", &status)); // consume 1 pose at a time
  TEST_ASSERT_EQUAL_STRING("", b.performPose("Z.", &status));
  TEST_ASSERT_EQUAL(NULL, b.performPose("", &status));

  // Move malformed (does not end in .)
  TEST_ASSERT_EQUAL_STRING("Z", b.performPose("Mp1.Z", &status)); // consume 1 pose at a time
  TEST_ASSERT_EQUAL(NULL, b.performPose("Z", &status));
}

void test_body_dances_are_valid() {

  Quotes q("q");
  Notifier n("n");
  n.setup(lcdImg, messageOnLcd);
  Images i("i");
  Ifttt it("m");

  Body b("b");
  initBody(&b, &q, &i, &it, &n);

  TEST_ASSERT_EQUAL_STRING("", lastArms);

  PoseExecStatus status;

  // Move correctly formed
  TEST_ASSERT_EQUAL(End, b.performMove("Mp1.Z."));
  TEST_ASSERT_EQUAL(End, b.performMove("D1."));
  TEST_ASSERT_EQUAL(End, b.performMove("A00."));
  TEST_ASSERT_EQUAL(End, b.performMove("A99."));
  TEST_ASSERT_EQUAL(End, b.performMove("B00."));
  TEST_ASSERT_EQUAL(End, b.performMove("B99."));
  TEST_ASSERT_EQUAL(End, b.performMove("C00."));
  TEST_ASSERT_EQUAL(End, b.performMove("C99."));

  TEST_ASSERT_EQUAL(End, b.performMove("Fw."));
  TEST_ASSERT_EQUAL(End, b.performMove("Fb."));
  TEST_ASSERT_EQUAL(End, b.performMove("Fa."));
  TEST_ASSERT_EQUAL(End, b.performMove("Fr."));
  TEST_ASSERT_EQUAL(End, b.performMove("Fl."));
  TEST_ASSERT_EQUAL(End, b.performMove("Fs."));
  TEST_ASSERT_EQUAL(End, b.performMove("FS."));
  TEST_ASSERT_EQUAL(End, b.performMove("Fn."));
  TEST_ASSERT_EQUAL(End, b.performMove("Fz."));
  TEST_ASSERT_EQUAL(End, b.performMove("F_."));
  TEST_ASSERT_EQUAL(End, b.performMove("F-."));
  TEST_ASSERT_EQUAL(End, b.performMove("F0."));
  TEST_ASSERT_EQUAL(End, b.performMove("F1."));
  TEST_ASSERT_EQUAL(End, b.performMove("F2."));
  TEST_ASSERT_EQUAL(End, b.performMove("F3."));

  TEST_ASSERT_EQUAL(End, b.performMove("Iiftt_action."));

  TEST_ASSERT_EQUAL(End, b.performMove("Lry."));
  TEST_ASSERT_EQUAL(End, b.performMove("Lrn."));
  TEST_ASSERT_EQUAL(End, b.performMove("Lrt."));
  TEST_ASSERT_EQUAL(End, b.performMove("Lwy."));
  TEST_ASSERT_EQUAL(End, b.performMove("Lyy."));
  TEST_ASSERT_EQUAL(End, b.performMove("Lyt."));
  TEST_ASSERT_EQUAL(End, b.performMove("L*y."));
  TEST_ASSERT_EQUAL(End, b.performMove("L*n."));
  TEST_ASSERT_EQUAL(End, b.performMove("Lfy."));
  TEST_ASSERT_EQUAL(End, b.performMove("L?."));

  TEST_ASSERT_EQUAL(End, b.performMove("Mc4."));
  TEST_ASSERT_EQUAL(End, b.performMove("Mk3."));
  TEST_ASSERT_EQUAL(End, b.performMove("Mp1."));
  TEST_ASSERT_EQUAL(End, b.performMove("Mq1."));
  TEST_ASSERT_EQUAL(End, b.performMove("M1onemessage."));

  TEST_ASSERT_EQUAL(End, b.performMove("W1."));

  TEST_ASSERT_EQUAL(End, b.performMove("Nnotification."));

  TEST_ASSERT_EQUAL(End, b.performMove("Z."));

  TEST_ASSERT_EQUAL(End, b.performMove("Dn."));
  TEST_ASSERT_EQUAL(End, b.performMove("Du."));
  TEST_ASSERT_EQUAL(End, b.performMove("D\\."));
  TEST_ASSERT_EQUAL(End, b.performMove("D/."));

  TEST_ASSERT_EQUAL(End, b.performMove("D0."));
  TEST_ASSERT_EQUAL(End, b.performMove("D1."));
  TEST_ASSERT_EQUAL(End, b.performMove("D2."));
  TEST_ASSERT_EQUAL(End, b.performMove("D3."));
}

void test_body_parses_move_timing_alias() {

  Quotes q("q");
  Notifier n("n");
  n.setup(lcdImg, messageOnLcd);
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
  RUN_TEST(test_body_dances_are_valid);
  return (UNITY_END());
}

#endif // UNIT_TEST
