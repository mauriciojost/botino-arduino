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
  LcdConfigChan0Light,
  LcdConfigChan1Light,
  LcdConfigChannel,
  LcdConfigStateDelimiter // delimiter of the configuration states
};


class Lcd : public Actor {

private:
  int channel;
  FreqConf freqConf;

  bool lightChan0;
  Buffer<LCD_LINE_LENGTH>* line0Chan0;
  Buffer<LCD_LINE_LENGTH>* line1Chan0;

  bool lightChan1;
  Buffer<LCD_LINE_LENGTH>* line0Chan1;
  Buffer<LCD_LINE_LENGTH>* line1Chan1;

public:

  Lcd(): freqConf(Never)  {
    channel = 0;
    line0Chan0 = new Buffer<LCD_LINE_LENGTH>("");
    line1Chan0 = new Buffer<LCD_LINE_LENGTH>("");
    line0Chan1 = new Buffer<LCD_LINE_LENGTH>("");
    line1Chan1 = new Buffer<LCD_LINE_LENGTH>("");
    lightChan0 = false;
    lightChan1 = false;
  }

  void initialize() { }

  const char *getName() { return "lcd"; }

  void cycle(bool cronMatches) { }

  void getActuatorValue(Value* value) { }

  const char* getPropName(int propIndex) {
    switch (propIndex) {
      case (LcdConfigChannel): return "chan";
      case (LcdConfigChan0Line0): return "c0l0";
      case (LcdConfigChan0Line1): return "c0l1";
      case (LcdConfigChan1Line0): return "c1l0";
      case (LcdConfigChan1Line1): return "c1l1";
      case (LcdConfigChan0Light): return "ligc0";
      case (LcdConfigChan1Light): return "ligc1";
      default: return "";
    }
  }

  void setProp(int propIndex, SetMode setMode, const Value* targetValue, Value* actualValue) {
    switch (propIndex) {
      case (LcdConfigChan0Line0):
        if (setMode == SetValue) {
          line0Chan0->load(targetValue);
        }
        if (actualValue != NULL) {
          actualValue->load(line0Chan0);
        }
        break;
      case (LcdConfigChan0Line1):
        if (setMode == SetValue) {
          line1Chan0->load(targetValue);
        }
        if (actualValue != NULL) {
          actualValue->load(line1Chan0);
        }
        break;
      case (LcdConfigChan1Line0):
        if (setMode == SetValue) {
          line0Chan1->load(targetValue);
        }
        if (actualValue != NULL) {
          actualValue->load(line0Chan1);
        }
        break;
      case (LcdConfigChan1Line1):
        if (setMode == SetValue) {
          line1Chan1->load(targetValue);
        }
        if (actualValue != NULL) {
          actualValue->load(line1Chan1);
        }
        break;
      case (LcdConfigChan0Light):
        if (setMode == SetValue) {
          Boolean b(targetValue);
          lightChan0 = b.get();
        }
        if (actualValue != NULL) {
          Boolean b(lightChan0);
          actualValue->load(&b);
        }
        break;
      case (LcdConfigChan1Light):
        if (setMode == SetValue) {
          Boolean b(targetValue);
          lightChan1 = b.get();
        }
        if (actualValue != NULL) {
          Boolean b(lightChan1);
          actualValue->load(&b);
        }
        break;
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

  int getNroProps() { return LcdConfigStateDelimiter; }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH>* info) { }

  int getNroInfos() { return 0; }

  FreqConf *getFrequencyConfiguration() { return &freqConf; }

  bool getLight() {
    if (channel % NRO_CHANNELS == 0) {
      return lightChan0;
    } else {
      return lightChan1;
    }
  }

  int getChannel() { return channel; }

};

#endif // LCD_INC
