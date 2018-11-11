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

#define EMPTY_NOTIF_REPRESENTATION ""
#define MAX_NRO_NOTIFS 4 // must be aligned with enum below

enum NotifierProps {
  NotifierFreqProp = 0,
  NotifierNotif0Prop,
  NotifierNotif1Prop,
  NotifierNotif2Prop,
  NotifierNotif3Prop,
  NotifierPropsDelimiter // count of properties
};

class Notifier : public Actor {

private:
  const char *name;
  Metadata *md;
  Queue<MAX_NRO_NOTIFS, MAX_NOTIF_LENGTH> queue;
  void (*messageFunc)(int x, int y, int color, bool wrap, bool clear, int size, const char *str);

  bool isInitialized() {
    return messageFunc != NULL;
  }

public:
  Notifier(const char *n) {
    name = n;
    messageFunc = NULL;
    md = new Metadata(n);
    md->getTiming()->setFreq("0");
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

    notify(); // apart from the message, also notify if notifications are available
  }

  int notification(const char *msg) {
    int i = queue.pushUnique(msg);
    log(CLASS_NOTIFIER, Debug, "New notif: %s (%d)", msg, i);
    return i;
  }

  const char *getNotification() {
    return queue.get();
  }

  int notificationRead() {
  	int i = queue.pop();
    log(CLASS_NOTIFIER, Debug, "Remove notif: %d", i);
    return i;
  }

  void act() {
    if (!isInitialized()) {
      log(CLASS_NOTIFIER, Warn, "No init!");
      return;
    }
    if (getTiming()->matches()) {
    	notify();
    }
  }

  void notify() {
    const char *currentNotif = getNotification();
    if (currentNotif != NULL) {
      log(CLASS_NOTIFIER, Debug, "Notif(%d): %s", queue.size(), currentNotif);
      Buffer aux(LCD_WIDTH);
        aux.fill("--(%d)--", queue.size());
        messageFunc(0, (NOTIF_LINE - 1) * 8 * NOTIF_SIZE, WHITE, DO_NOT_WRAP, DO_NOT_CLEAR, NOTIF_SIZE, aux.center(' ', LCD_WIDTH));
        aux.load(currentNotif);
        messageFunc(0, (NOTIF_LINE) * 8 * NOTIF_SIZE, WHITE, DO_NOT_WRAP, DO_NOT_CLEAR, NOTIF_SIZE, aux.center(' ', LCD_WIDTH));
    } else {
      log(CLASS_NOTIFIER, Debug, "No notifs");
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (NotifierFreqProp):
        return "freq";
      case (NotifierNotif0Prop):
        return "n0";
      case (NotifierNotif1Prop):
        return "n1";
      case (NotifierNotif2Prop):
        return "n2";
      case (NotifierNotif3Prop):
        return "n3";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    if (propIndex >= NotifierNotif0Prop && propIndex < (MAX_NRO_NOTIFS + NotifierNotif0Prop)) {
      int i = (int)propIndex - (int)NotifierNotif0Prop;
      if (m == SetCustomValue) {
        Buffer a = Buffer(MAX_NOTIF_LENGTH, targetValue);
        if (!a.isEmpty()) {
          queue.pushUnique(a.getBuffer());
        }
      }
      if (actualValue != NULL) {
        actualValue->load(queue.getAt(i, EMPTY_NOTIF_REPRESENTATION));
      }
    } else if (propIndex == NotifierFreqProp) {
      setPropTiming(m, targetValue, actualValue, md->getTiming());
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
