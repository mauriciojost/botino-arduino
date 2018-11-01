#ifndef NOTIFIER_INC
#define NOTIFIER_INC

/**
 * Notifier
 *
 * Responsible of making sure that the end user each and every message they should have got.
 * Handles the LCD too.
 */

#define CLASS_NOTIFIER "NF"
#define MAX_NOTIF_LENGTH 64
#define NOTIF_LINE 7
#define NOTIF_SIZE 1
#define BLACK 0
#define WHITE 1
#define DO_WRAP true
#define DO_NOT_WRAP false
#define DO_CLEAR true
#define DO_NOT_CLEAR false

#ifndef LCD_WIDTH
#define LCD_WIDTH 21
#endif // LCD_WIDTH

#include <main4ino/Actor.h>
#include <main4ino/Queue.h>

enum NotifierProps {
  NotifierFreqProp = 0,
  NotifierPropsDelimiter // count of properties
};

class Notifier : public Actor {

private:
  const char *name;
  Metadata *md;
  Queue<8, MAX_NOTIF_LENGTH> queue;
  void (*messageFunc)(int x, int y, int color, bool wrap, bool clear, int size, const char *str);

  bool isInitialized() {
    return messageFunc != NULL;
  }

public:
  Notifier(const char *n) {
    name = n;
    messageFunc = NULL;
    md = new Metadata(n);
    md->getTiming()->setFreq("300000005");
    notification("Welcome!");
  }

  const char *getName() {
    return name;
  }

  void setMessageFunc(void (*f)(int x, int y, int color, bool wrap, bool clear, int size, const char *str)) {
    messageFunc = f;
  }

  void message(int line, int size, const char *format, ...) {
    if (!isInitialized()) {
      log(CLASS_NOTIFIER, Warn, "No init!");
      return;
    }
    Buffer buffer(MAX_NOTIF_LENGTH - 1);
    va_list args;
    va_start(args, format);
    vsnprintf(buffer.getUnsafeBuffer(), MAX_NOTIF_LENGTH, format, args);
    buffer.getUnsafeBuffer()[MAX_NOTIF_LENGTH - 1] = 0;
	messageFunc(0, line * 8 * size, WHITE, DO_WRAP, DO_CLEAR, size, buffer.getBuffer());
    va_end(args);
  }

  int notification(const char *msg) {
    log(CLASS_NOTIFIER, Debug, "New notif: %s (%d + 1)", msg, queue.size());
    return queue.push(msg);
  }

  const char *getNotification() {
    return queue.get();
  }

  int notificationRead() {
    log(CLASS_NOTIFIER, Debug, "Remove notif: %d -1", queue.size());
    return queue.pop();
  }

  void act() {
    if (!isInitialized()) {
      log(CLASS_NOTIFIER, Warn, "No init!");
      return;
    }
    if (getTiming()->matches()) {
      const char *currentNotif = getNotification();
      if (currentNotif != NULL) {
    	  log(CLASS_NOTIFIER, Debug, "Notif(%d): %s", queue.size(), currentNotif);
    	  Buffer aux(LCD_WIDTH);
          aux.load("---");
          messageFunc(0, (NOTIF_LINE - 1) * 8 * NOTIF_SIZE, WHITE, DO_NOT_WRAP, DO_NOT_CLEAR, NOTIF_SIZE, aux.center(' ', LCD_WIDTH));
          aux.load(currentNotif);
          messageFunc(0, (NOTIF_LINE) * 8 * NOTIF_SIZE, WHITE, DO_NOT_WRAP, DO_NOT_CLEAR, NOTIF_SIZE, aux.center(' ', LCD_WIDTH));
      } else {
        log(CLASS_NOTIFIER, Debug, "No notifs");
      }
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (NotifierFreqProp):
        return "freq";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (NotifierFreqProp): {
        setPropTiming(m, targetValue, actualValue, md->getTiming());
      } break;
      default:
        break;
    }
    if (m != GetValue) {
      getMetadata()->changed();
    }
  }

  int getNroProps() {
    return NotifierPropsDelimiter;
  }

  void getInfo(int infoIndex, Buffer *info) {}

  int getNroInfos() {
    return 0;
  }

  Metadata *getMetadata() {
    return md;
  }
};

#endif // NOTIFIER_INC
