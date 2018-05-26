#ifndef HEXER_INC
#define HEXER_INC

#include <log4ino/Log.h>

#define CLASS_HEXER "HX"

/**
* This class is a helper for hexadecimal transformations.
*/
class Hexer {

private:
  static uint8_t hexToValue(char v) {
  	if (v >= '0' && v <= '9') {
  		return v - '0';
  	} else if (v >= 'a' && v <= 'f') {
  		return v - 'a' + 10;
  	} else if (v >= 'A' && v <= 'F') {
  		return v - 'A' + 10;
  	} else {
  		return 0;
  	}
  }

public:

  static void hexStrCpy(uint8_t* outputText, const char* inputHex) {
  	int l = strlen(inputHex);
    hexStrCpy(outputText, inputHex, l);
  }

  static void hexStrCpy(uint8_t* outputText, const char* inputHex, size_t l) {
  	if (l % 2 == 0) {
      int i;
      //log(CLASS_HEXER, Debug, "Parse hex: %s", inputHex);
  		for(i = 0; i < l; i = i + 2) {
  			outputText[i / 2] = hexToValue(inputHex[i]) * 16 + hexToValue(inputHex[i + 1]);
        //log(CLASS_HEXER, Debug, "  %.2x <- %c %c", outputText[i / 2], inputHex[i], inputHex[i+1]);
  		}
  		outputText[i / 2] = 0;
  	} else {
  		outputText[0] = 0;
  	}
  }

};

#endif // HEXER_INC
