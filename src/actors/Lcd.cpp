#include <actors/Lcd.h>


#define CLASS "LD"

Lcd::Lcd(): freqConf(Never)  {
  channel = 0;
  lineU = new Buffer<LCD_LINE_LENGTH>("");
  lineD = new Buffer<LCD_LINE_LENGTH>("");
  line0Chan0 = new Buffer<LCD_LINE_LENGTH>("");
  line1Chan0 = new Buffer<LCD_LINE_LENGTH>("");
  line0Chan1 = new Buffer<LCD_LINE_LENGTH>("");
  line1Chan1 = new Buffer<LCD_LINE_LENGTH>("");
}

void Lcd::initialize() { }

const char *Lcd::getName() {
  return "lcd";
}

void Lcd::cycle(bool cronMatches) { }

void Lcd::getActuatorValue(Value* value) { }

const char* Lcd::getPropName(int propIndex) {
  switch (propIndex) {
    case (LcdConfigChan0Line0): return "c0l0";
    case (LcdConfigChan0Line1): return "c0l1";
    case (LcdConfigChan1Line0): return "c1l0";
    case (LcdConfigChan1Line1): return "c1l1";
    case (LcdConfigChannel): return "chan";
    case (LcdConfigChan0Light): return "ligc0";
    case (LcdConfigChan1Light): return "ligc1";
    default: return "";
  }
}

void Lcd::setProp(int propIndex, SetMode setMode, const Value* targetValue, Value* actualValue) {
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

int Lcd::getNroProps() { return LcdConfigStateDelimiter; }

void Lcd::getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH>* info) {
}

int Lcd::getNroInfos() { return 0; }

FreqConf *Lcd::getFrequencyConfiguration() { return &freqConf; }

bool Lcd::getLight() {
  if (channel % NRO_CHANNELS == 0) {
    return lightChan0;
  } else {
    return lightChan1;
  }
}

int Lcd::getChannel() {
  return channel;
}
