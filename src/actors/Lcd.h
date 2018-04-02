#ifndef LCD_INC
#define LCD_INC

#include <log4ino/Log.h>
#include <main4ino/Misc.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>

#define CLASS_LCD "LD"
#define LCD_LINE_LENGTH 16
#define NRO_CHANNELS 2

enum LcdConfigState {
  LcdConfigChan0Line0 = 0,
  LcdConfigChan0Line1,
  LcdConfigChan1Line0,
  LcdConfigChan1Line1,
  LcdConfigChannel,
  LcdConfigStateDelimiter // delimiter of the configuration states
};

class Lcd : public Actor {

private:
  const char *name;
  int channel;
  Timing freqConf;

  Buffer<LCD_LINE_LENGTH> *line0Chan0;
  Buffer<LCD_LINE_LENGTH> *line1Chan0;

  Buffer<LCD_LINE_LENGTH> *line0Chan1;
  Buffer<LCD_LINE_LENGTH> *line1Chan1;

  void (*stdOutFunction)(int, const char *);

public:
  Lcd(const char *n) : freqConf(OnceEvery5Seconds) {
    name = n;
    channel = 0;
    line0Chan0 = new Buffer<LCD_LINE_LENGTH>("");
    line1Chan0 = new Buffer<LCD_LINE_LENGTH>("");
    line0Chan1 = new Buffer<LCD_LINE_LENGTH>("");
    line1Chan1 = new Buffer<LCD_LINE_LENGTH>("");
    stdOutFunction = NULL;
  }

  const char *getName() {
    return name;
  }

  void act() {
    if (freqConf.matches()) {
      if (stdOutFunction != NULL) {
        if (channel == 0) {
          stdOutFunction(0, line0Chan0->getBuffer());
          stdOutFunction(1, line1Chan0->getBuffer());
        } else {
          stdOutFunction(0, line0Chan1->getBuffer());
          stdOutFunction(1, line1Chan1->getBuffer());
        }
      }
    }
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (LcdConfigChannel):
        return "chan";
      case (LcdConfigChan0Line0):
        return "c0l0";
      case (LcdConfigChan0Line1):
        return "c0l1";
      case (LcdConfigChan1Line0):
        return "c1l0";
      case (LcdConfigChan1Line1):
        return "c1l1";
      default:
        return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value *targetValue, Value *actualValue) {
    switch (propIndex) {
      case (LcdConfigChan0Line0):
      	setPropValue(setMode, targetValue, actualValue, (Value*)line0Chan0); break;
      case (LcdConfigChan0Line1):
      	setPropValue(setMode, targetValue, actualValue, (Value*)line1Chan0); break;
      case (LcdConfigChan1Line0):
      	setPropValue(setMode, targetValue, actualValue, (Value*)line0Chan1); break;
      case (LcdConfigChan1Line1):
      	setPropValue(setMode, targetValue, actualValue, (Value*)line1Chan1); break;
      case (LcdConfigChannel):
        if (setMode == SetValue) {
          Integer i(targetValue);
          channel = i.get() % NRO_CHANNELS;
        }
        if (actualValue != NULL) {
          Integer i(channel);
          actualValue->load(&i);
        }
        break;
      default:
        break;
    }
  }

  int getNroProps() {
    return LcdConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &freqConf;
  }

  void setStdoutFunction(void (*stdOutWriteStringFunction)(int, const char *)) {
    stdOutFunction = stdOutWriteStringFunction;
  }
};

#endif // LCD_INC
