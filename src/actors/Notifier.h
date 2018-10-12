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
#define NOTIF_LINE 0
#define NOTIF_SIZE 1

#include <main4ino/Actor.h>
#include <main4ino/Queue.h>

class Notifier : public Actor {

private:

  const char *name;
  Metadata* md;
  Queue<8, MAX_NOTIF_LENGTH> queue;
  void (*messageFunc)(int line, const char *str, int size);

  bool isInitialized() {
    return messageFunc != NULL;
  }

public:

  Notifier(const char* n) {
    name = n;
    messageFunc = NULL;
    md = new Metadata(n);
    md->getTiming()->setFreq("300000005");
    notification("Welcome!");
  }

  const char *getName() {
    return name;
  }

  void setMessageFunc(void (*f)(int line, const char *str, int size)) {
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
    messageFunc(line, buffer.getBuffer(), size);
    va_end(args);
  }

  int notification(const char* msg) {
	log(CLASS_NOTIFIER, Debug, "New notif: %s", msg);
  	return queue.push(msg);
  }

  const char* getNotification() {
  	return queue.get();
  }

  int notificationRead() {
	log(CLASS_NOTIFIER, Debug, "Remove notif");
	return queue.pop();
  }

  void act() {
    if (!isInitialized()) {
      log(CLASS_NOTIFIER, Warn, "No init!");
      return;
    }
    if (getTiming()->matches()) {
      const char* currentNotif = getNotification();
      if (currentNotif != NULL) {
    	message(NOTIF_LINE, NOTIF_SIZE, "* %s *", currentNotif);
      } else {
    	log(CLASS_NOTIFIER, Debug, "No notifs");
      }
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) { }

  int getNroProps() {
    return 0;
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
