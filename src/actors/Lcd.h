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
  LiquidCrystal *lcd;
  int channel;
  FreqConf freqConf;

  bool light;
  Buffer<LCD_LINE_LENGTH>* lineU;
  Buffer<LCD_LINE_LENGTH>* lineD;

  bool lightChan0;
  Buffer<LCD_LINE_LENGTH>* line0Chan0;
  Buffer<LCD_LINE_LENGTH>* line1Chan0;

  bool lightChan1;
  Buffer<LCD_LINE_LENGTH>* line0Chan1;
  Buffer<LCD_LINE_LENGTH>* line1Chan1;

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

  bool getLight();

  int getChannel();

};

#endif // LCD_INC
