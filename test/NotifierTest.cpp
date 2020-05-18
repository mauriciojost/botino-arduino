#ifdef UNIT_TEST

// Auxiliary libraries
#include <unity.h>

// Being tested
#include <actors/Notifier.h>

char message[64];
char image;

void setUp() {
  image = 'U'; // not initialized
}

void tearDown() {}

void lcdImg(char img, uint8_t bitmap[]) {
  image = img;
}

void messageFunc(int x, int y, int color, bool wrap, MsgClearMode clear, int size, const char *str) {
  sprintf(message, "%s", str);
}

void test_basic_behaviour() {
  Notifier n("n1");
  n.setup(lcdImg, messageFunc);

  n.notification("wakeup");
  TEST_ASSERT_EQUAL_STRING("     (1) wakeup     ", message);
  TEST_ASSERT_EQUAL('U', image); // not initialized

  n.notificationRead();
  TEST_ASSERT_EQUAL_STRING("         <>         ", message);
  TEST_ASSERT_EQUAL('U', image); // not initialized
}

void test_serialization_deserialization() {
  Notifier n("n1");
  n.setup(lcdImg, messageFunc);

  TEST_ASSERT_EQUAL(NULL, n.getNotification()); // no notifs

  RichBuffer b("notif1:notif2::");

  n.bufferToQueue(&b);

  n.queueToBuffer(&b);

  TEST_ASSERT_EQUAL_STRING("notif1:notif2::", b.getBuffer()->getBuffer());

  TEST_ASSERT_EQUAL_STRING("notif1", n.getNotification());
  TEST_ASSERT_EQUAL(1, n.notificationRead());
  TEST_ASSERT_EQUAL_STRING("notif2", n.getNotification());
  TEST_ASSERT_EQUAL(0, n.notificationRead());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_basic_behaviour);
  RUN_TEST(test_serialization_deserialization);
  return (UNITY_END());
}

#endif // UNIT_TEST
