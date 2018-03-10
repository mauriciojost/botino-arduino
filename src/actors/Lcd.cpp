#include <actors/Lcd.h>


#define CLASS "Lcd"

#define PERIOD_FORCE_UPDATE 20

byte modeButtonIcon[8] = {
    B11111,
    B11011,
    B11101,
    B00000,
    B11101,
    B11011,
    B11111,
};

Lcd::Lcd(int rsPin, int enablePin, int d4Pin, int d5Pin, int d6Pin, int d7Pin): freqConf(Never)  {
  channel = 0;
  lcd = new LiquidCrystal(rsPin, enablePin, d4Pin, d5Pin, d6Pin, d7Pin);
  lineU = new Buffer<LCD_LINE_LENGTH>("");
  lineD = new Buffer<LCD_LINE_LENGTH>("");
  line0Chan0 = new Buffer<LCD_LINE_LENGTH>("");
  line1Chan0 = new Buffer<LCD_LINE_LENGTH>("");
  line0Chan1 = new Buffer<LCD_LINE_LENGTH>("");
  line1Chan1 = new Buffer<LCD_LINE_LENGTH>("");
}

void Lcd::initialize() {
  lcd->begin(16, 2);
  lcd->noAutoscroll();
  lcd->leftToRight();
  lcd->noBlink();
  lcd->createChar(1, modeButtonIcon); // will be printed whenever character \1 is used
  lcd->clear();
}

void Lcd::display(const char *strUp, const char *strDown) {
  static int updates = 0;
  updates = (updates + 1) % PERIOD_FORCE_UPDATE;
  bool forceFullUpdate = updates == 0;
  if (forceFullUpdate) {
    initialize(); // did not find a way a better way to ensure LCD won't get
                  // corrupt due to load noise (if any)
  }
  if (strUp != NULL) {
    if (strcmp(strUp, lineU->getBuffer()) || forceFullUpdate) {
      lineU->load(strUp);
      lcd->setCursor(0, 0);
      lcd->print(strUp);
      log(CLASS, Debug, "Set: ", strUp);
    } else {
      log(CLASS, Debug, "Keep: ", strUp);
    }
  }
  if (strDown != NULL) {
    if (strcmp(strDown, lineD->getBuffer()) || forceFullUpdate) {
      lineD->load(strDown);
      lcd->setCursor(0, 1);
      lcd->print(strDown);
      log(CLASS, Debug, "Set: ", strDown);
    } else {
      log(CLASS, Debug, "Keep: ", strDown);
    }
  }
}

const char *Lcd::getName() {
  return "lcd";
}

void Lcd::cycle(bool cronMatches) {
  if (channel % NRO_CHANNELS == 0) {
    display(line0Chan0->getBuffer(), line1Chan0->getBuffer());
  } else {
    display(line0Chan1->getBuffer(), line1Chan1->getBuffer());
  }
}

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
