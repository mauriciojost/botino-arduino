#ifndef LCD_INC
#define LCD_INC

#include <log4ino/Log.h>
#include <main4ino/Misc.h>
#include <main4ino/Actor.h>
#include <main4ino/Value.h>
#include <main4ino/Integer.h>
#include <main4ino/Boolean.h>
#include <LiquidCrystal.h>
#include <Wire.h>

#define LCD_LINE_LENGTH 16

enum LcdConfigState {
  LcdConfigLineUpAState = 0,
  LcdConfigLineDownAState,
  LcdConfigLineUpBState,
  LcdConfigLineDownBState,
  LcdConfigChannelState,
  LcdConfigStateDelimiter // delimiter of the configuration states
};


class Lcd : public Actor {

private:
  LiquidCrystal *lcd;
  int updates;
  int channel;
  FreqConf freqConf;
  Buffer<LCD_LINE_LENGTH>* lineU;
  Buffer<LCD_LINE_LENGTH>* lineD;
  Buffer<LCD_LINE_LENGTH>* lineUA;
  Buffer<LCD_LINE_LENGTH>* lineDA;
  Buffer<LCD_LINE_LENGTH>* lineUB;
  Buffer<LCD_LINE_LENGTH>* lineDB;

  void display(const char *upLine, const char *downLine);

public:
  Lcd(int rsPin, int enablePin, int d4Pin, int d5Pin, int d6Pin, int d7Pin);
  void initialize();

  const char *getName();

  void cycle(bool cronMatches);
  void getActuatorValue(Value* value);

  int getNroProps();
  const char* getPropName(int propIndex);
  void setProp(int propIndex, SetMode set, const Value* targetValue, Value* actualValue);

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH>* info);
  int getNroInfos();

  FreqConf *getFrequencyConfiguration();

};

#endif // LCD_INC
