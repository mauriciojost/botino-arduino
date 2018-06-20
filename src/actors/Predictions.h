#ifndef PREDICTIONS_INC
#define PREDICTIONS_INC

#include <log4ino/Log.h>
#include <main4ino/Buffer.h>
#include <main4ino/Misc.h>

#define CLASS_PREDICTIONS "PR"

/**
 * Generates funny predictions.
 */

#define RANDOM random(10000)

class Predictions {

private:
public:
  static const char *getPrediction(Buffer<200> *msg) {
    msg->fill("%s will %s %s %s %s", getObject(), getTransitiveVerb(), getObject(), getWhen(), getWhere());
  }

  static const char *getObject() {
    return getObject(RANDOM);
  }
  static const char *getObject(int i) {
    switch (i % 26) {
      case 0:
        return "your colleague";
      case 1:
        return "you";
      case 2:
        return "your brother";
      case 3:
        return "your sister";
      case 4:
        return "your couple";
      case 5:
        return "a banana";
      case 6:
        return "the person next to you";
      case 7:
        return "the person in front of you";
      case 8:
        return "the person behind you";
      case 9:
        return "your dog";
      case 10:
        return "your window";
      case 11:
        return "your car";
      case 12:
        return "your laptop";
      case 13:
        return "your table";
      case 14:
        return "your chair";
      case 15:
        return "an elephant";
      case 16:
        return "a palace";
      case 17:
        return "a bitcoin";
      case 18:
        return "a coconut";
      case 19:
        return "a car";
      case 20:
        return "a lollipop";
      case 21:
        return "a window";
      case 22:
        return "a beer";
      case 23:
        return "batman";
      case 24:
        return "superman";
      case 25:
        return "a bug";
      default:
        return "?";
    }
  }

  static const char *getTransitiveVerb() {
    return getTransitiveVerb(RANDOM);
  }
  static const char *getTransitiveVerb(int i) {
    switch (i % 13) {
      case 0:
        return "ride";
      case 1:
        return "buy";
      case 2:
        return "cook";
      case 3:
        return "drink";
      case 4:
        return "eat";
      case 5:
        return "lick";
      case 6:
        return "call";
      case 7:
        return "commit";
      case 8:
        return "give away";
      case 9:
        return "steal";
      case 10:
        return "insult";
      case 11:
        return "pee";
      case 12:
        return "";
      default:
        return "?";
    }
  }

  static const char *getWhen() {
    return getWhen(RANDOM);
  }
  static const char *getWhen(int i) {
    switch (i % 4) {
      case 0:
        return "in 5 minutes";
      case 1:
        return "in 1 hour";
      case 2:
        return "today";
      case 3:
        return "tomorrow";
      default:
        return "?";
    }
  }

  static const char *getWhere() {
    return getWhere(RANDOM);
  }
  static const char *getWhere(int i) {
    switch (i % 12) {
      case 0:
        return "at work";
      case 1:
        return "under the table";
      case 2:
        return "in the shuttle";
      case 3:
        return "at the park";
      case 4:
        return "in a restaurant";
      case 5:
        return "in the toilet";
      case 6:
        return "in the shower";
      case 7:
        return "in the jungle";
      case 8:
        return "in the post office";
      case 9:
        return "in the office";
      case 10:
        return "at mom's";
      case 11:
        return "at mom's";
      default:
        return "?";
    }
  }
};

#endif // PREDICTIONS_INC
