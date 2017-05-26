#include <actors/Lcd.h>


#define CLASS "Lcd"

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
  lcd = new LiquidCrystal(rsPin, enablePin, d4Pin, d5Pin, d6Pin, d7Pin);
  lineUp = new Buffer<LCD_LINE_LENGTH>("");
  lineDown = new Buffer<LCD_LINE_LENGTH>("");
}

void Lcd::initialize() {
  lcd->begin(16, 2);
  lcd->noAutoscroll();
  lcd->leftToRight();
  lcd->noBlink();
  lcd->createChar(1, modeButtonIcon); // will be printed whenever character \1 is used
  lcd->clear();
}

void Lcd::display(const char *str1, const char *str2) {
  updates++;
  bool onceInAWhile = ((updates % 20) == 0);
  if (onceInAWhile) {
    initialize(); // did not find a way a better way to ensure LCD won't get
                  // corrupt due to load noise
  }
  lcd->setCursor(0, 0);
  lcd->print(str1);
  lcd->setCursor(0, 1);
  lcd->print(str2);
}

const char *Lcd::getName() {
  return "lcd";
}

void Lcd::cycle(bool cronMatches) { }

void Lcd::subCycle(float subCycle) { }

void Lcd::getActuatorValue(Value* value) {
  value->load(lineUp);
}

const char* Lcd::getPropName(int propIndex) {
  switch (propIndex) {
    case (LcdConfigLineUpState): return "lineup";
    case (LcdConfigLineDownState): return "linedown";
    default: return "";
  }
}

void Lcd::setProp(int propIndex, SetMode setMode, const Value* targetValue, Value* actualValue) {
  switch (propIndex) {
    case (LcdConfigLineUpState):
      if (setMode == SetValue) {
        lineUp->load(targetValue);
        display(lineUp->getBuffer(), lineDown->getBuffer());
      }
      if (actualValue != NULL) {
        actualValue->load(lineUp);
      }
      break;
    case (LcdConfigLineDownState):
      if (setMode == SetValue) {
        lineDown->load(targetValue);
        display(lineUp->getBuffer(), lineDown->getBuffer());
      }
      if (actualValue != NULL) {
        actualValue->load(lineDown);
      }
      break;
    default:
      break;
  }
}

int Lcd::getNroProps() { return LcdConfigStateDelimiter; }

void Lcd::getInfo(int infoIndex, Buffer<MAX_VALUE_STR_LENGTH>* info) {
}

int Lcd::getNroInfos() { return 0; }

FreqConf *Lcd::getFrequencyConfiguration() { return &freqConf; }
